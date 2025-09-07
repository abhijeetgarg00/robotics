#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>

#include <gz/transport/Node.hh>
#include <gz/msgs/pose_v.pb.h>

class GzModelTFBroadcaster : public rclcpp::Node
{
public:
  GzModelTFBroadcaster()
  : Node("gz_model_tf_broadcaster")
  {
    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);

    // Subscribe directly to Gazebo transport
    if (!gz_node_.Subscribe("/world/empty/pose/info",
                            &GzModelTFBroadcaster::poseCallback, this))
    {
      RCLCPP_ERROR(this->get_logger(), "Failed to subscribe to /world/empty/pose/info");
    }
    else
    {
      RCLCPP_INFO(this->get_logger(), "Subscribed to /world/empty/pose/info");
    }
  }

private:
  void poseCallback(const gz::msgs::Pose_V &msg)
  {
    for (int i = 0; i < msg.pose_size(); ++i)
    {
      const auto &pose = msg.pose(i);
      if (pose.name() == "workpiece_box")
      {
        geometry_msgs::msg::TransformStamped t;
        t.header.stamp = this->now();
        t.header.frame_id = "world";
        t.child_frame_id = "workpiece_box";
        t.transform.translation.x = pose.position().x();
        t.transform.translation.y = pose.position().y();
        t.transform.translation.z = pose.position().z();
        t.transform.rotation.x = pose.orientation().x();
        t.transform.rotation.y = pose.orientation().y();
        t.transform.rotation.z = pose.orientation().z();
        t.transform.rotation.w = pose.orientation().w();

        tf_broadcaster_->sendTransform(t);
      }
    }
  }

  gz::transport::Node gz_node_;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<GzModelTFBroadcaster>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
