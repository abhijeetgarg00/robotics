#include <rclcpp/rclcpp.hpp>
#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/solvers/pipeline_planner.h>
#include <moveit/task_constructor/stages/current_state.h>
#include <moveit/task_constructor/stages/move_to.h>

using namespace moveit::task_constructor;

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("mtc_minimal");

  // Create a task
  Task task("minimal_task");
  task.loadRobotModel(node);
  task.setProperty("group", "panda_arm");

  // Create a pipeline planner (OMPL)
  auto pipeline = std::make_shared<solvers::PipelinePlanner>(node);
  pipeline->setPlannerId("ompl", "RRTConnectkConfigDefault");

  // Stage 0: Current state
  task.add(std::make_unique<stages::CurrentState>("current"));

  // Stage 1: Move to named state from SRDF (e.g., "ready")
  {
    auto move = std::make_unique<stages::MoveTo>("move to ready", pipeline);
    move->setGroup("panda_arm");
    move->setGoal("ready");   // must exist in your panda.srdf
    task.add(std::move(move));
  }

  // Plan
  if (task.plan() != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(node->get_logger(), "Planning failed");
    rclcpp::shutdown();
    return 1;
  }

  // Get first solution
  const auto& sols = task.solutions();
  if (sols.empty()) {
    RCLCPP_ERROR(node->get_logger(), "No solutions found.");
    rclcpp::shutdown();
    return 1;
  }

  // Publish solution to RViz
  task.introspection().publishSolution(*sols.front());
  RCLCPP_INFO(node->get_logger(), "Published plan to RViz.");

  // (optional) tiny wait to ensure ExecuteTaskSolution server is available
  rclcpp::sleep_for(std::chrono::milliseconds(500));

  // Execute solution via move_group
  auto exec_result = task.execute(*sols.front());
  if (exec_result != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(node->get_logger(), "Execution failed");
    rclcpp::shutdown();
    return 2;
  }

  RCLCPP_INFO(node->get_logger(), "Executed task solution successfully!");
  rclcpp::shutdown();
  return 0;
}
