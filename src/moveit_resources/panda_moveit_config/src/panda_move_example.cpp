#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <moveit/move_group_interface/move_group_interface.h>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("panda_move_example");
  rclcpp::executors::SingleThreadedExecutor exec;
  exec.add_node(node);
  std::thread spinner([&exec]() { exec.spin(); });

  static const std::string PLANNING_GROUP = "panda_arm";
  moveit::planning_interface::MoveGroupInterface move_group(node, PLANNING_GROUP);

  // Optional tuning
  move_group.setPlanningTime(5.0);
  move_group.setMaxVelocityScalingFactor(0.3);
  move_group.setMaxAccelerationScalingFactor(0.3);
  move_group.setEndEffectorLink("panda_link8");        // tip link of the arm
  move_group.setPoseReferenceFrame("panda_link0");     // matches your RViz/virtual joint

  RCLCPP_INFO(node->get_logger(), "Planning frame: %s", move_group.getPlanningFrame().c_str());
  RCLCPP_INFO(node->get_logger(), "End effector link: %s", move_group.getEndEffectorLink().c_str());

  // Always start from the robot's current state
  move_group.setStartStateToCurrentState();

  // ---- Target Pose #1 ----
  geometry_msgs::msg::Pose pose1;
  pose1.orientation.w = 1.0;           // identity orientation
  pose1.position.x = 0.40;
  pose1.position.y = 0.00;
  pose1.position.z = 0.40;

  move_group.setPoseTarget(pose1);

  moveit::planning_interface::MoveGroupInterface::Plan plan1;
  bool ok = (move_group.plan(plan1) == moveit::core::MoveItErrorCode::SUCCESS);
  if (!ok)
  {
    RCLCPP_ERROR(node->get_logger(), "Planning to pose #1 failed");
    rclcpp::shutdown();
    spinner.join();
    return 1;
  }
  RCLCPP_INFO(node->get_logger(), "Executing to pose #1...");
  move_group.execute(plan1);

  // Small pause
  rclcpp::sleep_for(std::chrono::milliseconds(500));

  // ---- Target Pose #2 ----
  geometry_msgs::msg::Pose pose2;
  pose2.orientation.w = 1.0;           // keep same orientation
  pose2.position.x = 0.30;
  pose2.position.y = 0.20;
  pose2.position.z = 0.25;

  move_group.setStartStateToCurrentState();  // update start state
  move_group.setPoseTarget(pose2);

  moveit::planning_interface::MoveGroupInterface::Plan plan2;
  ok = (move_group.plan(plan2) == moveit::core::MoveItErrorCode::SUCCESS);
  if (!ok)
  {
    RCLCPP_ERROR(node->get_logger(), "Planning to pose #2 failed");
    rclcpp::shutdown();
    spinner.join();
    return 1;
  }
  RCLCPP_INFO(node->get_logger(), "Executing to pose #2...");
  move_group.execute(plan2);

  RCLCPP_INFO(node->get_logger(), "Done!");
  rclcpp::shutdown();
  spinner.join();
  return 0;
}
