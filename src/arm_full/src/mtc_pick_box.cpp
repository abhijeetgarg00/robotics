#include <rclcpp/rclcpp.hpp>

#include <moveit/task_constructor/task.h>
#include <moveit/task_constructor/stages/current_state.h>
#include <moveit/task_constructor/stages/move_to.h>
#include <moveit/task_constructor/stages/move_relative.h>
#include <moveit/task_constructor/stages/modify_planning_scene.h>
#include <moveit/task_constructor/solvers/pipeline_planner.h>
#include <moveit/task_constructor/solvers/cartesian_path.h>

#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit_msgs/msg/collision_object.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <shape_msgs/msg/solid_primitive.hpp>

using moveit::task_constructor::Task;
namespace stages  = moveit::task_constructor::stages;
namespace solvers = moveit::task_constructor::solvers;

static const std::string ARM_GROUP = "panda_arm";
static const std::string HAND_GROUP = "hand";
static const std::string EEF_LINK   = "panda_hand";
static const std::string OBJECT_ID  = "workpiece_box";

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("mtc_pick_box");

  // Allow overriding object pose (defaults match your SDF center @ z=0.60)
  const double obj_x = node->declare_parameter("box_x", 0.50);
  const double obj_y = node->declare_parameter("box_y", 0.00);
  const double obj_z = node->declare_parameter("box_z", 0.60);

  // --------------------------------------------------
  // Add the box to the PlanningScene (simple 20x20x10 cm)
  // --------------------------------------------------
  moveit::planning_interface::PlanningSceneInterface psi;

  moveit_msgs::msg::CollisionObject box;
  box.header.frame_id = "world";
  box.id = OBJECT_ID;

  shape_msgs::msg::SolidPrimitive prim;
  prim.type = shape_msgs::msg::SolidPrimitive::BOX;
  prim.dimensions.resize(3);
  prim.dimensions[shape_msgs::msg::SolidPrimitive::BOX_X] = 0.20;
  prim.dimensions[shape_msgs::msg::SolidPrimitive::BOX_Y] = 0.20;
  prim.dimensions[shape_msgs::msg::SolidPrimitive::BOX_Z] = 0.10;

  geometry_msgs::msg::Pose box_pose;
  box_pose.position.x = obj_x;
  box_pose.position.y = obj_y;
  // place so the base touches z = obj_z - 0.05
  box_pose.position.z = obj_z - 0.10 / 2.0;
  box_pose.orientation.w = 1.0;

  box.primitives.push_back(prim);
  box.primitive_poses.push_back(box_pose);
  box.operation = box.ADD;

  psi.applyCollisionObject(box);

  // --------------------------------------------------
  // Build MTC Task
  // --------------------------------------------------
  Task task("pick_box");
  task.loadRobotModel(node);
  task.setProperty("group", ARM_GROUP);
  task.setProperty("eef", HAND_GROUP);
  task.setProperty("ik_frame", EEF_LINK);

  // Solvers
  auto pipeline = std::make_shared<solvers::PipelinePlanner>(node);
  pipeline->setPlannerId("ompl", "RRTConnectkConfigDefault");

  auto cartesian = std::make_shared<solvers::CartesianPath>();
  cartesian->setMaxVelocityScalingFactor(0.2);
  cartesian->setMaxAccelerationScalingFactor(0.2);
  cartesian->setStepSize(0.002);

  // 0) Current state
  task.add(std::make_unique<stages::CurrentState>("current"));

  // 1) Allow contact between EEF and object while grasping
  {
    auto allow = std::make_unique<stages::ModifyPlanningScene>("allow eef-object contact");
    allow->allowCollisions(OBJECT_ID, { EEF_LINK }, true);
    task.add(std::move(allow));
  }

  // 2) Open gripper (SRDF group_state "open")
  {
    auto open = std::make_unique<stages::MoveTo>("open hand", pipeline);
    open->setGroup(HAND_GROUP);
    open->setGoal("open");
    task.add(std::move(open));
  }

  // 3) Move to pregrasp pose
  {
    geometry_msgs::msg::PoseStamped pre;
    pre.header.frame_id = "world";
    pre.pose.position.x = obj_x + 0.15;   // 15 cm in front of box
    pre.pose.position.y = obj_y;
    pre.pose.position.z = obj_z;          // level with box center

    // RPY(0,0,pi) -> (0,0,1,0). Fingers close along +/-Y.
    pre.pose.orientation.x = 0.0;
    pre.pose.orientation.y = 0.0;
    pre.pose.orientation.z = 1.0;
    pre.pose.orientation.w = 0.0;

    auto move_pre = std::make_unique<stages::MoveTo>("move to pregrasp", pipeline);
    move_pre->setGroup(ARM_GROUP);
    move_pre->setIKFrame(EEF_LINK);
    move_pre->setGoal(pre);
    task.add(std::move(move_pre));
  }

  // 4) Cartesian approach along -X
  {
    auto approach = std::make_unique<stages::MoveRelative>("approach", cartesian);
    approach->setGroup(ARM_GROUP);
    approach->setIKFrame(EEF_LINK);

    geometry_msgs::msg::Vector3Stamped dir;
    dir.header.frame_id = "world";
    dir.vector.x = -1.0; dir.vector.y = 0.0; dir.vector.z = 0.0;
    approach->setDirection(dir);

    // No setDistance() in your API → use properties
    approach->properties().set("min_distance", 0.08);
    approach->properties().set("max_distance", 0.12);
    task.add(std::move(approach));
  }

  // 5) Close gripper
  {
    auto close = std::make_unique<stages::MoveTo>("close hand", pipeline);
    close->setGroup(HAND_GROUP);
    close->setGoal("close");
    task.add(std::move(close));
  }

  // 6) Attach object to the hand
  {
    auto attach = std::make_unique<stages::ModifyPlanningScene>("attach");
    attach->attachObject(OBJECT_ID, EEF_LINK);
    task.add(std::move(attach));
  }

  // 7) Lift along +Z
  {
    auto lift = std::make_unique<stages::MoveRelative>("lift", cartesian);
    lift->setGroup(ARM_GROUP);
    lift->setIKFrame(EEF_LINK);

    geometry_msgs::msg::Vector3Stamped up;
    up.header.frame_id = "world";
    up.vector.x = 0.0; up.vector.y = 0.0; up.vector.z = 1.0;
    lift->setDirection(up);

    lift->properties().set("min_distance", 0.05);
    lift->properties().set("max_distance", 0.15);
    task.add(std::move(lift));
  }

  // --------------------------------------------------
  // Plan & Execute
  // --------------------------------------------------
  auto plan_result = task.plan();
  if (plan_result != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(node->get_logger(), "MTC planning failed: %d", plan_result.val);
    rclcpp::shutdown();
    return 1;
  }

  // Execute best (first) solution
  const auto& sols = task.solutions();
  if (sols.empty()) {
    RCLCPP_ERROR(node->get_logger(), "No solutions generated");
    rclcpp::shutdown();
    return 2;
  }

  auto exec_result = task.execute(*sols.front());
  if (exec_result != moveit::core::MoveItErrorCode::SUCCESS) {
    RCLCPP_ERROR(node->get_logger(), "MTC execution failed: %d", exec_result.val);
    rclcpp::shutdown();
    return 3;
  }

  RCLCPP_INFO(node->get_logger(), "Pick complete");
  rclcpp::shutdown();
  return 0;
}
