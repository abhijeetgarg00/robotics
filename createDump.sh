find /home/beast/ros2_ws/src/arm_full \
  -path "/home/beast/ros2_ws/src/arm_full/meshes" -prune -o \
  -path "/home/beast/ros2_ws/src/arm_full/arm_full" -prune -o \
  -path "/home/beast/ros2_ws/src/arm_full/resource" -prune -o \
  -path "/home/beast/ros2_ws/src/arm_full/test" -prune -o \
  -path "*/__pycache__*" -prune -o \
  -type f -print | while read file; do
    echo "$file"
    cat "$file"
    echo    # blank line separator
done > /home/beast/ros2_ws/_chatgpt/arm_full_dump.txt