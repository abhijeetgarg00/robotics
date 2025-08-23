# ROS 2 Quick Reference Notes

## 1. Publisher-Subscriber Demo

# Start a publisher node (talker)
ros2 run demo_nodes_cpp talker

# Start a subscriber node (listener)
ros2 run demo_nodes_cpp listener

# Visualize nodes and topics
rqt_graph

---

## 2. Turtlesim Demo

# Launch the turtlesim simulator
ros2 run turtlesim turtlesim_node

# Control the turtle with keyboard
ros2 run turtlesim turtle_teleop_key

---

## 3. Environment Setup

# Source ROS 2 installation
source /opt/ros/jazzy/setup.bash

# Source colcon autocomplete (optional, for convenience)
source /usr/share/colcon_argcomplete/hook/colcon-argcomplete.bash

# Source your workspace's setup script (after building)
source ~/code/ros2_ws/install/setup.bash

---

## 4. Workspace and Package Creation

# Create a new workspace (if not already created)
mkdir -p ~/code/ros2_ws/src
cd ~/code/ros2_ws

# Build the workspace
colcon build

# Create a new Python package with dependencies
ros2 pkg create my_robot_controller --build-type ament_python --dependencies rclpy

# build the package
colcon build


There is file name , node name, and executable name 

my_first_node, first_node, test_node

all three can be and should be different


# when we are building and continue testing
# WE DONT HAVE RUN BUILD AGAIN AND AGAIN
colcon build --symlink-install

---

## 5. Useful Tips

# To see all active nodes
ros2 node list

# To see all topics
ros2 topic list

# To echo messages from a topic
ros2 topic echo /topic_name

# To get info about a topic
ros2 topic info /topic_name

# to get interface information
ros2 interface show <interfaceName>
ros2 interface show std_msgs/msg/String

# get service information 
ros2 service list
ros2 service type /add_two_ints

#Examples for service

ros2 run demo_nodes_cpp add_two_ints_server

ros2 service call /add_two_ints example_interfaces/srv/AddTwoInts "{'a':2, 'b': 5}"


# how to  start action
ros2 run my_robot_controller fibonacci_action_server

# How to call on a action 
ros2 action send_goal /fibonacci example_interfaces/action/Fibonacci "{order: 10}" --feedback


# parameters
ros2 param list
ros2 param get /param_node speed
ros2 param set /param_node speed 5.5

# Launch file can be created to start the node need to be part of packages

# there is something called quality of service, where we can define different 
# stratigy like, history of publisher and if we should drop packages or not executable

# Learning tf2 or transform systems
ros2 run tf2_ros static_transform_publisher 2 1 0 0.785 0 0 world robot_1
ros2 run tf2_ros static_transform_publisher 2 0 0 0 0 0 robot_1 robot_2

# urdf and rviz visulization of the robot_1
ros2 run robot_state_publisher robot_state_publisher   --ros-args -p robot_description:="$(xacro $(ros2 pkg prefix moveit_resources_panda_description)/share/moveit_resources_panda_description/urdf/panda.urdf.xacro hand:=true)"
ros2 run joint_state_publisher_gui joint_state_publisher_gui
ros2 run tf2_tools view_frames.py

## 6. ROS 2 Control

ROS 2 Control is a framework for real-time robot control in ROS 2. It standardizes how robot hardware (like joints, sensors, and actuators) interacts with controllers and the rest of the ROS 2 ecosystem.

### Key Definitions

- **Hardware Interface**: The abstraction layer between ROS 2 and robot hardware. It exposes:
    - **Command Interfaces**: For sending commands (read/write) to hardware.
    - **State Interfaces**: For reading the current state (read-only) from hardware.
- **Controller**: A software component that implements a control algorithm (e.g., position, velocity, or effort control).
- **Controller Manager**: Manages the lifecycle of controllers (loading, starting, stopping).

### Architecture Diagram



- Command interfaces (read/write) send commands to the hardware.
- State interfaces (read-only) report the hardware state back to ROS 2.

For more details, see the [ros2_control documentation](https://control.ros.org/).


