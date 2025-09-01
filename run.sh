clear

cd ~/ros2_ws
colcon build --symlink-install
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 launch arm_full gazebo_moveit.launch.py ros2_control_hardware_type:=gz