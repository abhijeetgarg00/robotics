#!/usr/bin/env python3
import time
import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer
from example_interfaces.action import Fibonacci

class FibServer(Node):
    def __init__(self):
        super().__init__('fibonacci_action_server')
        self._server = ActionServer(self, Fibonacci, 'fibonacci', self.execute_cb)

    def execute_cb(self, goal_handle):
        n = max(0, int(goal_handle.request.order))
        self.get_logger().info(f'Executing goal: order={n}')

        # Build initial sequence (length 0, 1, or 2 depending on n)
        seq = []
        if n >= 1:
            seq = [0]
        if n >= 2:
            seq = [0, 1]

        feedback = Fibonacci.Feedback()

        # Publish initial feedback
        feedback.sequence = seq[:]
        goal_handle.publish_feedback(feedback)

        # Grow sequence, publish feedback each step
        while len(seq) < n:
            if goal_handle.is_cancel_requested:
                goal_handle.canceled()
                self.get_logger().info('Goal canceled')
                result = Fibonacci.Result()
                result.sequence = seq
                return result

            if len(seq) < 2:
                seq.append(1 if len(seq) == 1 else 0)
            else:
                seq.append(seq[-1] + seq[-2])

            feedback.sequence = seq[:]
            goal_handle.publish_feedback(feedback)
            time.sleep(0.2)   # simulate work

        goal_handle.succeed()
        result = Fibonacci.Result()
        result.sequence = seq
        return result

def main(args=None):
    rclpy.init(args=args)
    node = FibServer()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()