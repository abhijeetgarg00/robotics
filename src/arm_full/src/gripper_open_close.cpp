#include <chrono>
#include <map>
#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.h>  // .h is fine too

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("gripper_open_close");

  moveit::planning_interface::MoveGroupInterface gripper(node, "hand");

  std::map<std::string, double> target_open{
      {"panda_finger_joint1", 0.035},
      {"panda_finger_joint2", 0.035}};
  gripper.setJointValueTarget(target_open);
  gripper.move();

  rclcpp::sleep_for(std::chrono::seconds(2));

  std::map<std::string, double> target_close{
      {"panda_finger_joint1", 0.0},
      {"panda_finger_joint2", 0.0}};
  gripper.setJointValueTarget(target_close);
  gripper.move();

  rclcpp::shutdown();
  return 0;
}
