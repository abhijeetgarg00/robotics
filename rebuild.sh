# from your workspace root
rm -rf build/arm_full install/arm_full log
rm -rf src/arm_full/*.egg-info  # important for setuptools file lists


cd ~/ros2_ws
colcon build --symlink-install
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash