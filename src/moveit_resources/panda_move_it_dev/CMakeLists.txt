cmake_minimum_required(VERSION 3.10)
project(panda_move_it_dev)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)

add_executable(panda_move_example src/panda_move_example.cpp)
ament_target_dependencies(panda_move_example
  rclcpp
  geometry_msgs
  moveit_ros_planning_interface
)

install(TARGETS panda_move_example
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_package()
