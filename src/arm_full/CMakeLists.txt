cmake_minimum_required(VERSION 3.8)
project(arm_full)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(moveit_task_constructor_core REQUIRED)
find_package(moveit_task_constructor_msgs REQUIRED)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(moveit_ros_planning_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(class_loader REQUIRED)

# ---- C++ node ----

add_executable(mtc_minimal src/mtc_minimal.cpp)
ament_target_dependencies(mtc_minimal
  rclcpp
  moveit_task_constructor_core
  moveit_task_constructor_msgs
)
install(TARGETS mtc_minimal DESTINATION lib/${PROJECT_NAME})


add_executable(mtc_pick_box src/mtc_pick_box.cpp)
ament_target_dependencies(mtc_pick_box
  rclcpp
  moveit_task_constructor_core
  moveit_task_constructor_msgs
)


add_executable(gripper_open_close src/gripper_open_close.cpp)
ament_target_dependencies(gripper_open_close
  rclcpp
  moveit_ros_planning_interface
)

install(TARGETS mtc_pick_box
  DESTINATION lib/${PROJECT_NAME})

install(TARGETS gripper_open_close
  DESTINATION lib/${PROJECT_NAME})

# ---- Python (reuse setup.py) ----
# # ament_python_install_package(${PROJECT_NAME})
# install(PROGRAMS
#   ${PROJECT_NAME}/nodes/gripper_open_close.py
#   DESTINATION lib/${PROJECT_NAME}
# )

# ---- Share resources (urdf, meshes, launch, etc.) ----
install(DIRECTORY launch urdf config meshes worlds models
  DESTINATION share/${PROJECT_NAME}
)

# ---- Export ----
ament_package()
