clear

colcon build --packages-select arm_full
source install/setup.bash
ros2 run arm_full gripper_open_close
