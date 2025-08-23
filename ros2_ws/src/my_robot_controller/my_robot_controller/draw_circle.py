#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist


class DrawCircle(Node):
    def __init__(self):
        super().__init__('draw_circle')

        self.cmd_vel_publisher_ = self.create_publisher(Twist, '/turtle1/cmd_vel', 10)
        self.timer = self.create_timer(1.0, self.send_velocity_command)
        self.get_logger().info('Draw Circle Node has been started.')

    def send_velocity_command(self):
        msg = Twist()
        msg.linear.x = 10.0
        msg.angular.z = 7.8
        self.cmd_vel_publisher_.publish(msg)
        self.get_logger().info('Publishing velocity command to draw a circle.')
        
def main(args=None):
    rclpy.init(args=args)
    draw_circle_node = DrawCircle()
    rclpy.spin(draw_circle_node)
    draw_circle_node.destroy_node()
    rclpy.shutdown()


