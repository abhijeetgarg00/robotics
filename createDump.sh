find /home/beast/ros2_ws/src/moveit_resources/panda_moveit_config \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_description/meshes" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/launch/demo.launch.py" -prune -o \
  -type f -print | while read file; do
    echo "$file"
    cat "$file"
    echo    # blank line separator
done > /home/beast/ros2_ws/_chatgpt/arm_full_dump.txt

cd ~/ros2_ws && colcon build --packages-select moveit_resources_panda_moveit_config && source install/setup.bash
ros2 run moveit_resources_panda_moveit_config panda_move_example