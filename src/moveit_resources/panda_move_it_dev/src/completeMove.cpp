// src/move_above_box_align.cpp
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>
#include <moveit/move_group_interface/move_group_interface.hpp>

static double yawFromQuat(const geometry_msgs::msg::Quaternion& q_msg) {
  tf2::Quaternion q; tf2::fromMsg(q_msg, q);
  double r,p,y; tf2::Matrix3x3(q).getRPY(r,p,y); return y;
}

static bool planToPose(moveit::planning_interface::MoveGroupInterface& arm,
                       const geometry_msgs::msg::PoseStamped& target)
{
  arm.setStartStateToCurrentState();
  arm.setPoseTarget(target);
  moveit::planning_interface::MoveGroupInterface::Plan plan;
  if (arm.plan(plan) != moveit::core::MoveItErrorCode::SUCCESS) return false;
  return arm.execute(plan) == moveit::core::MoveItErrorCode::SUCCESS;
}

int main(int argc, char** argv){
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("move_above_box_align");
  rclcpp::executors::SingleThreadedExecutor exec; exec.add_node(node);
  std::thread spinner([&exec](){ exec.spin(); });

  auto tf_buffer = std::make_shared<tf2_ros::Buffer>(node->get_clock());
  auto tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

  moveit::planning_interface::MoveGroupInterface arm(node, "panda_arm");
  moveit::planning_interface::MoveGroupInterface hand(node, "hand");
  arm.setPoseReferenceFrame("panda_link0");
  arm.setEndEffectorLink("panda_link8");           // or your TCP
  arm.setPlanningTime(15.0);
  arm.setNumPlanningAttempts(10);
  arm.setMaxVelocityScalingFactor(0.3);
  arm.setMaxAccelerationScalingFactor(0.3);
  arm.setGoalOrientationTolerance(0.10);
  arm.setGoalPositionTolerance(0.01);
  arm.setPlanningPipelineId("ompl");
  arm.setPlannerId("ompl::RRTConnectkConfigDefault");

  // TFs
  geometry_msgs::msg::TransformStamped tf_box, tf_notch;
  try{
    tf_box   = tf_buffer->lookupTransform("panda_link0","workpiece_box",  tf2::TimePointZero,std::chrono::seconds(2));
    tf_notch = tf_buffer->lookupTransform("panda_link0","workpiece_notch",tf2::TimePointZero,std::chrono::seconds(2));
  }catch(const tf2::TransformException& ex){
    RCLCPP_ERROR(node->get_logger(),"TF lookup failed: %s", ex.what());
    rclcpp::shutdown(); spinner.join(); return 1;
  }

  // Geometry (match your collision object)
  const double box_h=0.10, notch_h=0.06, notch_z_local=0.08;
  const double top_surface = std::max(0.5*box_h, notch_z_local + 0.5*notch_h); // 0.11
  const double clearance = 0.20;
  const double z_goal = tf_box.transform.translation.z + top_surface + clearance;

  const double yaw_notch = yawFromQuat(tf_notch.transform.rotation);
  const double x_goal = tf_box.transform.translation.x;
  const double y_goal = tf_box.transform.translation.y;

  // Orientation: EXACT notch yaw, tool-down
  tf2::Quaternion qz,qx,q; qz.setRPY(0,0,yaw_notch + M_1_PI/4); qx.setRPY(M_PI,0,0); q = qz*qx; q.normalize();

  // ---- 1) Move above notch (pose goal) ----
  geometry_msgs::msg::PoseStamped pose_above;
  pose_above.header.stamp = node->now();
  pose_above.header.frame_id = "panda_link0";
  pose_above.pose.position.x = x_goal;
  pose_above.pose.position.y = y_goal;
  pose_above.pose.position.z = z_goal;
  pose_above.pose.orientation = tf2::toMsg(q);

  if (!planToPose(arm, pose_above)) {
    RCLCPP_ERROR(node->get_logger(),"Plan to above notch failed");
    rclcpp::shutdown(); spinner.join(); return 2;
  }

  // ---- 2) Open gripper ----
  hand.setNamedTarget("open"); hand.move();

  // ---- 3) Descend (normal plan to lower pose) ----
  const double descend = 0.1; // 6 cm
  geometry_msgs::msg::PoseStamped pose_down = pose_above;
  pose_down.pose.position.z = z_goal - descend; // keep same orientation
  if (!planToPose(arm, pose_down)) {
    RCLCPP_ERROR(node->get_logger(),"Plan descend failed");
    rclcpp::shutdown(); spinner.join(); return 3;
  }

  // ---- 4) Close gripper (pick) ----
  hand.setNamedTarget("close"); hand.move();

  // ---- 5) Lift back up (normal plan) ----
  geometry_msgs::msg::PoseStamped pose_lift = pose_above; // back to above height
  if (!planToPose(arm, pose_lift)) {
    RCLCPP_ERROR(node->get_logger(),"Plan lift failed");
    rclcpp::shutdown(); spinner.join(); return 4;
  }

  // ---- 6) Rotate joint1 to +90° ----
  {
    std::vector<double> joints = arm.getCurrentJointValues(); // [j1..j7]
    joints[0] = M_PI/2.0;
    arm.setJointValueTarget(joints);
    if (arm.move() != moveit::core::MoveItErrorCode::SUCCESS) {
      RCLCPP_ERROR(node->get_logger(),"Rotate joint1 failed");
      rclcpp::shutdown(); spinner.join(); return 5;
    }
  }

  // ---- 7) Descend to place ----
  geometry_msgs::msg::PoseStamped pose_place_down = arm.getCurrentPose();
  pose_place_down.header.frame_id = "panda_link0";
  pose_place_down.pose.orientation = tf2::toMsg(q); // keep tool-down yaw (optional)
  pose_place_down.pose.position.z -= (descend + 0.02);
  if (!planToPose(arm, pose_place_down)) {
    RCLCPP_ERROR(node->get_logger(),"Plan place descend failed");
    rclcpp::shutdown(); spinner.join(); return 6;
  }

  // ---- 8) Open to release ----
  hand.setNamedTarget("open"); hand.move();

  // ---- 9) Lift a bit after placing ----
  geometry_msgs::msg::PoseStamped pose_retreat = pose_place_down;
  pose_retreat.pose.position.z += 0.10;
  planToPose(arm, pose_retreat);

  RCLCPP_INFO(node->get_logger(),"Pick → rotate → place: done ✔");
  rclcpp::shutdown(); spinner.join(); return 0;
}
