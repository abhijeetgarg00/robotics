cmake_minimum_required(VERSION 3.10)
project(panda_move_it_dev)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gz-transport13 REQUIRED)

add_executable(panda_move_example src/panda_move_example.cpp)
ament_target_dependencies(panda_move_example
  rclcpp
  geometry_msgs
  moveit_ros_planning_interface
)
install(TARGETS panda_move_example
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

add_executable(pick_open src/pick_open.cpp)
ament_target_dependencies(pick_open
  rclcpp
  geometry_msgs
  tf2
  tf2_geometry_msgs
  moveit_ros_planning_interface
)
install(TARGETS pick_open
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

add_executable(gz_model_tf_broadcaster src/gz_model_tf_broadcaster.cpp)
ament_target_dependencies(gz_model_tf_broadcaster
  rclcpp
  geometry_msgs
  tf2_ros
)
target_link_libraries(gz_model_tf_broadcaster
  gz-transport13::core
  ${Protobuf_LIBRARIES}
)
install(TARGETS gz_model_tf_broadcaster
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# NEW: addCollision
add_executable(addCollision src/addCollision.cpp)
ament_target_dependencies(addCollision
  rclcpp
  geometry_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  moveit_ros_planning_interface
)
install(TARGETS addCollision
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

add_executable(completeMove src/completeMove.cpp)
ament_target_dependencies(completeMove
  rclcpp
  geometry_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  moveit_ros_planning_interface
)
install(TARGETS completeMove
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)


# Install the launch files if you want them available
install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME})

ament_package()
