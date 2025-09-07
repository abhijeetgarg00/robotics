clear

colcon build --symlink-install
source install/setup.bash
ros2 run panda_move_it_dev gz_model_tf_broadcaster