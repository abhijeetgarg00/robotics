#!/usr/bin/env python3
import rclpy
from rclpy.node import Node

class ParamNode(Node):
    def __init__(self):
        super().__init__('param_node')

        # Declare parameter with default value
        self.declare_parameter('speed', 1.0)

        # Timer that checks parameter every second
        self.timer = self.create_timer(1.0, self.timer_cb)

    def timer_cb(self):
        # Read current parameter value
        speed = self.get_parameter('speed').get_parameter_value().double_value
        self.get_logger().info(f'Current speed: {speed}')

def main(args=None):
    rclpy.init(args=args)
    node = ParamNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()