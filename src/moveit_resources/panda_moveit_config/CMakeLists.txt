cmake_minimum_required(VERSION 3.10.2)
project(moveit_resources_panda_moveit_config)
find_package(ament_cmake REQUIRED)

ament_package()

install(DIRECTORY launch DESTINATION share/${PROJECT_NAME})
install(DIRECTORY config DESTINATION share/${PROJECT_NAME})
install(FILES .setup_assistant DESTINATION share/${PROJECT_NAME})

find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)

add_executable(panda_move_example src/panda_move_example.cpp)
ament_target_dependencies(panda_move_example
  rclcpp
  geometry_msgs
  moveit_ros_planning_interface
)
install(TARGETS panda_move_example DESTINATION lib/${PROJECT_NAME})