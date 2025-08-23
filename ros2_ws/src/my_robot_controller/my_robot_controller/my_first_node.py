#!/usr/bin/env python3
import rclpy
from rclpy.node import Node


class MyNode(Node):
    def __init__(self):
        super().__init__('first_node')

        self.counter_ = 0
        # Create a timer that triggers every second
        self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        self.get_logger().info('Timer callback triggered! ' + str(self.counter_))
        self.counter_ += 1




def main(args=None):
    rclpy.init(args=args)

    # Create a node
    node = MyNode()

    # Spin the node so it can process callbacks and keep running
    rclpy.spin(node)

    # Shutdown the node and clean up
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()