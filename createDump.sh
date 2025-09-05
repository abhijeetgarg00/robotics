find /home/beast/ros2_ws/src/ \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_description/meshes" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/launch/demo.launch.py" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/launch/moveit_empty.rviz" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/launch/moveit_rviz.launch.py" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/config/bio_ik_kinematics.yaml" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/config/chomp_planning.yaml" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/config/panda.srdf" -prune -o \
  -path "/home/beast/ros2_ws/src/moveit_resources/panda_moveit_config/config/panda_arm.srdf.xacro" -prune -o \
  -type f -print | while read file; do
    echo "$file"
    cat "$file"
    echo    # blank line separator
done > /home/beast/ros2_ws/_chatgpt/arm_full_dump.txt