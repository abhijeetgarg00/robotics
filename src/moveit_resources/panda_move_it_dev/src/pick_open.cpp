#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <moveit/move_group_interface/move_group_interface.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("panda_pick_open");

  // Spin in background so MoveGroupInterface & TF can work
  rclcpp::executors::SingleThreadedExecutor exec;
  exec.add_node(node);
  std::thread spinner([&exec]() { exec.spin(); });

  // TF setup
  auto tf_buffer = std::make_shared<tf2_ros::Buffer>(node->get_clock());
  auto tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

  // Arm + hand groups
  static const std::string ARM_GROUP = "panda_arm";
  static const std::string HAND_GROUP = "hand";
  moveit::planning_interface::MoveGroupInterface arm(node, ARM_GROUP);
  moveit::planning_interface::MoveGroupInterface hand(node, HAND_GROUP);

  arm.setPoseReferenceFrame("panda_link0");
  arm.setEndEffectorLink("panda_link8");
  arm.setPlanningTime(5.0);
  arm.setMaxVelocityScalingFactor(0.2);
  arm.setMaxAccelerationScalingFactor(0.2);

  // --- Look up TF for box ---
  geometry_msgs::msg::TransformStamped tf_box;
  try {
    tf_box = tf_buffer->lookupTransform("panda_link0", "workpiece_box", tf2::TimePointZero, std::chrono::seconds(2));
  } catch (const tf2::TransformException &ex) {
    RCLCPP_ERROR(node->get_logger(), "Could not get TF: %s", ex.what());
    rclcpp::shutdown();
    spinner.join();
    return 1;
  }

  // --- Build pick pose just above box ---
  tf2::Quaternion q;
  q.setRPY(M_PI, 0.0, 0.0);  // tool down
  geometry_msgs::msg::PoseStamped pick_pose;
  pick_pose.header.stamp = node->now();
  pick_pose.header.frame_id = "panda_link0";
  pick_pose.pose.position.x = tf_box.transform.translation.x;
  pick_pose.pose.position.y = tf_box.transform.translation.y;
  pick_pose.pose.position.z = tf_box.transform.translation.z + 15;  // 10 cm above
  pick_pose.pose.orientation = tf2::toMsg(q);

  // --- Move arm ---
  arm.setPoseTarget(pick_pose);
  moveit::planning_interface::MoveGroupInterface::Plan plan;
  if (arm.plan(plan) == moveit::core::MoveItErrorCode::SUCCESS) {
    if (arm.execute(plan) == moveit::core::MoveItErrorCode::SUCCESS) {
      RCLCPP_INFO(node->get_logger(), "Arm moved above box ✔");
    } else {
      RCLCPP_ERROR(node->get_logger(), "Arm execution failed");
    }
  } else {
    RCLCPP_ERROR(node->get_logger(), "Arm planning failed");
  }

  // --- Open gripper ---
  hand.setNamedTarget("open");
  if (hand.move() == moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_INFO(node->get_logger(), "Gripper opened ✔");
  } else {
    RCLCPP_ERROR(node->get_logger(), "Gripper open failed");
  }

  rclcpp::shutdown();
  spinner.join();
  return 0;
}
