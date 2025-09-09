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
#include <thread>

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

  // Spin so TF lookups keep working
  rclcpp::executors::SingleThreadedExecutor exec;
  exec.add_node(node);
  std::thread spinner([&exec]() { exec.spin(); });

  // TF setup
  auto tf_buffer   = std::make_shared<tf2_ros::Buffer>(node->get_clock());
  auto tf_listener = std::make_shared<tf2_ros::TransformListener>(*tf_buffer);

  // MoveIt planning scene interface
  moveit::planning_interface::PlanningSceneInterface psi;

  const std::string world_frame = "panda_link0";
  const std::string model_frame = "workpiece_box";
  const std::string obj_id      = "workpiece_box";

  // Define primitives (match SDF)
  shape_msgs::msg::SolidPrimitive box;
  box.type = shape_msgs::msg::SolidPrimitive::BOX;
  box.dimensions = {0.20, 0.20, 0.10}; // x,y,z

  shape_msgs::msg::SolidPrimitive notch;
  notch.type = shape_msgs::msg::SolidPrimitive::BOX;
  notch.dimensions = {0.03, 0.05, 0.06}; // x,y,z

  // Local poses in the model frame
  geometry_msgs::msg::Pose lp_box;
  lp_box.orientation.w = 1.0; // identity
  lp_box.position.x = 0.0; lp_box.position.y = 0.0; lp_box.position.z = 0.0;

  geometry_msgs::msg::Pose lp_notch = lp_box;
  lp_notch.position.z = 0.08; // centered 8 cm above the box center (top face is +0.05)

  rclcpp::Rate rate(10.0); // ~10 Hz
  bool first_publish = true;

  while (rclcpp::ok())
  {
    try {
      // Lookup transform world<-model
      geometry_msgs::msg::TransformStamped tfm =
          tf_buffer->lookupTransform(world_frame, model_frame, tf2::TimePointZero, std::chrono::milliseconds(200));

      tf2::Transform root_tf;
      tf2::fromMsg(tfm.transform, root_tf);

      // Compose world poses for both primitives
      geometry_msgs::msg::Pose wp_box   = composePose(root_tf, lp_box);
      geometry_msgs::msg::Pose wp_notch = composePose(root_tf, lp_notch);

      moveit_msgs::msg::CollisionObject co;
      co.header.frame_id = world_frame;
      co.id = obj_id;

      co.primitives.push_back(box);
      co.primitives.push_back(notch);
      co.primitive_poses.push_back(wp_box);
      co.primitive_poses.push_back(wp_notch);

      // Use ADD each cycle; same id replaces/updates the object
      co.operation = moveit_msgs::msg::CollisionObject::ADD;

      psi.applyCollisionObject(co);

      if (first_publish) {
        RCLCPP_INFO(node->get_logger(), "Collision object '%s' added & syncing to TF '%s' -> '%s'",
                    obj_id.c_str(), world_frame.c_str(), model_frame.c_str());
        first_publish = false;
      }
    }
    catch (const tf2::TransformException& ex) {
      RCLCPP_WARN_THROTTLE(node->get_logger(), *node->get_clock(), 2000,
                           "TF lookup failed (%s -> %s): %s", world_frame.c_str(), model_frame.c_str(), ex.what());
    }

    rate.sleep();
  }

  rclcpp::shutdown();
  spinner.join();
  return 0;
}
