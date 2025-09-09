// src/addCollision.cpp
#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <shape_msgs/msg/solid_primitive.hpp>
#include <moveit/planning_scene_interface/planning_scene_interface.hpp>
#include <moveit_msgs/msg/collision_object.hpp>

#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Transform.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <algorithm>
#include <thread>
#include <chrono>

static geometry_msgs::msg::Pose composePose(const tf2::Transform& root_tf,
                                            const geometry_msgs::msg::Pose& local)
{
  tf2::Transform local_tf;
  tf2::fromMsg(local, local_tf);
  tf2::Transform world_tf = root_tf * local_tf;

  geometry_msgs::msg::Pose out;
  const tf2::Vector3& t = world_tf.getOrigin();
  const tf2::Quaternion& r = world_tf.getRotation();
  out.position.x = t.x();
  out.position.y = t.y();
  out.position.z = t.z();
  out.orientation = tf2::toMsg(r);
  return out;
}

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = rclcpp::Node::make_shared("add_collision_sync");

  // Executor for callbacks (TF, etc.)
  rclcpp::executors::SingleThreadedExecutor exec;
  exec.add_node(node);
  std::thread spinner([&exec]() { exec.spin(); });

  // TF
  auto tf_buffer   = std::make_shared<tf2_ros::Buffer>(node->get_clock());
  auto tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

  // MoveIt scene
  moveit::planning_interface::PlanningSceneInterface psi;

  const std::string world_frame = "panda_link0";
  const std::string model_frame = "workpiece_box";
  const std::string obj_id      = "workpiece_box";

  // Primitives (match SDF)
  shape_msgs::msg::SolidPrimitive box;
  box.type = shape_msgs::msg::SolidPrimitive::BOX;
  box.dimensions = {0.20, 0.20, 0.10};

  shape_msgs::msg::SolidPrimitive notch;
  notch.type = shape_msgs::msg::SolidPrimitive::BOX;
  notch.dimensions = {0.03, 0.05, 0.06};

  // Local poses in model frame
  geometry_msgs::msg::Pose lp_box;
  lp_box.orientation.w = 1.0;
  lp_box.position.x = 0.0; lp_box.position.y = 0.0; lp_box.position.z = 0.0;

  geometry_msgs::msg::Pose lp_notch = lp_box;
  lp_notch.position.z = 0.08;

  rclcpp::Rate rate(10.0);
  bool first_publish = true;
  bool running = true;

  // Main loop
  while (rclcpp::ok() && running)
  {
    try {
      auto tfm = tf_buffer->lookupTransform(world_frame, model_frame, tf2::TimePointZero,
                                            std::chrono::milliseconds(200));
      tf2::Transform root_tf;
      tf2::fromMsg(tfm.transform, root_tf);

      auto wp_box   = composePose(root_tf, lp_box);
      auto wp_notch = composePose(root_tf, lp_notch);

      moveit_msgs::msg::CollisionObject co;
      co.header.frame_id = world_frame;
      co.id = obj_id;
      co.primitives = {box, notch};
      co.primitive_poses = {wp_box, wp_notch};
      co.operation = moveit_msgs::msg::CollisionObject::ADD;

      psi.applyCollisionObject(co);

      if (first_publish) {
        RCLCPP_INFO(node->get_logger(),
                    "Collision object '%s' added & syncing to TF '%s' -> '%s'",
                    obj_id.c_str(), world_frame.c_str(), model_frame.c_str());
        first_publish = false;
      }
    } catch (const tf2::TransformException& ex) {
      RCLCPP_WARN_THROTTLE(node->get_logger(), *node->get_clock(), 2000,
                           "TF lookup failed (%s -> %s): %s",
                           world_frame.c_str(), model_frame.c_str(), ex.what());
    }

    rate.sleep();
    // If you want to break on some condition, set running=false;
  }

  // --- Graceful removal while node/executor are still alive ---
  RCLCPP_INFO(node->get_logger(), "Removing collision object '%s' ...", obj_id.c_str());

  // Explicit REMOVE op
  {
    moveit_msgs::msg::CollisionObject co;
    co.header.frame_id = world_frame;
    co.id = obj_id;
    co.operation = moveit_msgs::msg::CollisionObject::REMOVE;
    psi.applyCollisionObject(co);
  }

  // Also call convenience remover
  psi.removeCollisionObjects({obj_id});

  // Allow processing time
  std::this_thread::sleep_for(std::chrono::milliseconds(750));

  // Optional verify & retry
  {
    auto names = psi.getKnownObjectNames();
    if (std::find(names.begin(), names.end(), obj_id) != names.end()) {
      RCLCPP_WARN(node->get_logger(), "Object '%s' still present; retrying remove", obj_id.c_str());
      psi.removeCollisionObjects({obj_id});
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  rclcpp::shutdown();
  spinner.join();
  return 0;
}
