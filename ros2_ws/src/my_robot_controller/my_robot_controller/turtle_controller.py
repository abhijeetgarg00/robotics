#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from turtlesim.msg import Pose
from geometry_msgs.msg import Twist
from turtlesim.srv import SetPen
from functools import partial   

class TurtleControllerNode(Node):
    def __init__(self):
        super().__init__("turtle_controller")
        self.pub = self.create_publisher(Twist, "/turtle1/cmd_vel", 10)
        self.sub = self.create_subscription(Pose, "/turtle1/pose", self.pose_cb, 10)
        self.get_logger().info("Turtle controller has been started.")

        # Early-turn box (turtlesim is ~0..11). Adjust as you like.
        self.x_min, self.x_max = 2.0, 9.0
        self.y_min, self.y_max = 2.0, 9.0

        # Speeds
        self.v_fast = 5.0
        self.v_slow = 1.0
        self.w_turn = 0.9

    # def pose_cb(self, pose: Pose):
    #     cmd = Twist()
    #     # If near any wall, start turning (simple rule)
    #     if pose.x > self.x_max or pose.x < self.x_min or pose.y > self.y_max or pose.y < self.y_min:
    #         cmd.linear.x = self.v_slow
    #         cmd.angular.z = self.w_turn
    #     else:
    #         cmd.linear.x = self.v_fast
    #         cmd.angular.z = 0.0
    #     self.pub.publish(cmd)

    def set_pen(self, r, g, b, width, off):
        client = self.create_client(SetPen, "/turtle1/set_pen")
        if not client.wait_for_service(timeout_sec=1.0):
            self.get_logger().warn("SetPen service not available!")
            return
        req = SetPen.Request()
        req.r = r
        req.g = g
        req.b = b
        req.width = width
        req.off = off
        future = client.call_async(req)
        future.add_done_callback(partial(self.callback_set_pen))
        # Optionally, add a callback or handle the future
    
    def callback_set_pen(self, future):
        try:
            response = future.result()
        except Exception as e:
            self.get_logger().error(f"Service call failed: {e}")    


    def pose_cb(self, pose: Pose):
        cmd = Twist()
        # Pen color logic
        if pose.x < (self.x_min + self.x_max) / 2:
            self.set_pen(255, 0, 0, 3, 0)  # Red
        else:
            self.set_pen(0, 0, 255, 3, 0)  # Blue

        if pose.x > self.x_max or pose.x < self.x_min or pose.y > self.y_max or pose.y < self.y_min:
            cmd.linear.x = self.v_slow
            cmd.angular.z = self.w_turn
        else:
            cmd.linear.x = self.v_fast
            cmd.angular.z = 0.0
        self.pub.publish(cmd)

def main(args=None):
    rclpy.init(args=args)
    node = TurtleControllerNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == "__main__":
    main()