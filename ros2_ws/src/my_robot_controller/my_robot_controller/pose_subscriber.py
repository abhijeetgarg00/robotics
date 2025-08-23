#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from turtlesim.msg import Pose

class PoseSubscriber(Node):
    def __init__(self):
        super().__init__('pose_subscriber')
        self.subscription = self.create_subscription(
            Pose,
            '/turtle1/pose',
            self.listener_callback,
            10)
        self.pose = None
        self.timer = self.create_timer(1.0, self.timer_callback)

    def listener_callback(self, msg):
        self.pose = msg

    def timer_callback(self):
        if self.pose is not None:
            self.get_logger().info(
                f'x: {self.pose.x:.2f}, y: {self.pose.y:.2f}, theta: {self.pose.theta:.2f}, '
                f'linear_velocity: {self.pose.linear_velocity:.2f}, angular_velocity: {self.pose.angular_velocity:.2f}'
            )

def main(args=None):
    rclpy.init(args=args)
    node = PoseSubscriber()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()