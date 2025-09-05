clear

colcon build --symlink-install
source install/setup.bash

# Fake mode (sanity check)
#ros2 launch moveit_resources_panda_moveit_config gazebo_moveit.launch.py ros2_control_hardware_type:=mock_components

# Gazebo mode
ros2 launch moveit_resources_panda_moveit_config gazebo_moveit.launch.py ros2_control_hardware_type:=gz
