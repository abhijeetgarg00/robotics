#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <moveit/move_group_interface/move_group_interface.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("panda_pick_open");

  // Params for the pick pose (in panda_link0 frame)
  node->declare_parameter<double>("pick_x", 0.40);
  node->declare_parameter<double>("pick_y", 0.00);
  node->declare_parameter<double>("pick_z", 0.10);      // height of flange (link8)
  node->declare_parameter<double>("roll", 3.14159);     // tool pointing down (π about X)
  node->declare_parameter<double>("pitch", 0.0);
  node->declare_parameter<double>("yaw", 0.0);
  node->declare_parameter<double>("vel_scale", 0.2);
  node->declare_parameter<double>("acc_scale", 0.2);
  node->declare_parameter<double>("planning_time", 5.0);

  double px, py, pz, r, p, y, vs, as, pt;
  node->get_parameter("pick_x", px);
  node->get_parameter("pick_y", py);
  node->get_parameter("pick_z", pz);
  node->get_parameter("roll", r);
  node->get_parameter("pitch", p);
  node->get_parameter("yaw", y);
  node->get_parameter("vel_scale", vs);
  node->get_parameter("acc_scale", as);
  node->get_parameter("planning_time", pt);

  // Spin a thread so MoveGroupInterface can communicate with move_group
  rclcpp::executors::SingleThreadedExecutor exec;
  exec.add_node(node);
  std::thread spinner([&exec]() { exec.spin(); });

  // Arm + gripper groups from your SRDF
  static const std::string ARM_GROUP = "panda_arm";
  static const std::string HAND_GROUP = "hand";
  moveit::planning_interface::MoveGroupInterface arm(node, ARM_GROUP);
  moveit::planning_interface::MoveGroupInterface hand(node, HAND_GROUP);

  // Basic tuning
  arm.setPlanningTime(pt);
  arm.setMaxVelocityScalingFactor(vs);
  arm.setMaxAccelerationScalingFactor(as);
  arm.setPoseReferenceFrame("panda_link0"); // your virtual joint anchors link0 to world
  arm.setEndEffectorLink("panda_link8");    // flange; good for approach alignment

  // Build target pose
  tf2::Quaternion q;
  q.setRPY(r, p, y);
  geometry_msgs::msg::PoseStamped pick_pose;
  pick_pose.header.stamp = node->now();
  pick_pose.header.frame_id = "panda_link0";
  pick_pose.pose.position.x = px;
  pick_pose.pose.position.y = py;
  pick_pose.pose.position.z = pz;
  pick_pose.pose.orientation = tf2::toMsg(q);

  // Plan & execute to pick pose
  arm.setPoseTarget(pick_pose);
  moveit::planning_interface::MoveGroupInterface::Plan plan;
  bool ok = (arm.plan(plan) == moveit::core::MoveItErrorCode::SUCCESS);
  if (!ok) {
    RCLCPP_ERROR(node->get_logger(), "Planning to pick pose FAILED");
    rclcpp::shutdown();
    spinner.join();
    return 1;
  }
  auto exec_res = arm.execute(plan);
  if (exec_res != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(node->get_logger(), "Execution to pick pose FAILED");
    rclcpp::shutdown();
    spinner.join();
    return 2;
  }
  RCLCPP_INFO(node->get_logger(), "Reached pick pose ✔");

  // Open gripper using SRDF named state "open"
  hand.setNamedTarget("open");
  auto gr_exec = hand.move();  // internally plans+executes
  if (gr_exec != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(node->get_logger(), "Gripper open FAILED");
    rclcpp::shutdown();
    spinner.join();
    return 3;
  }
  RCLCPP_INFO(node->get_logger(), "Gripper opened ✔");

  rclcpp::shutdown();
  spinner.join();
  return 0;
}
