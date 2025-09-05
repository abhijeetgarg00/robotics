clear

cd ~/ros2_ws
colcon build --symlink-install --packages-select arm_full
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash
ros2 launch arm_full demo.launch.py