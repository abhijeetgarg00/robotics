clear

colcon build --symlink-install --packages-select panda_move_it_dev
source install/setup.bash
ros2 run panda_move_it_dev addCollision