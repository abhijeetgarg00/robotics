# Dockerfile
FROM osrf/ros:jazzy-desktop

# Core tools + RViz/MoveIt + Gazebo + Wayland support
RUN apt-get update && apt-get install -y \
    git curl vim bash-completion build-essential cmake \
    python3-pip python3-colcon-common-extensions mesa-utils \
    ros-jazzy-moveit ros-jazzy-moveit-resources-panda-moveit-config \
    ros-jazzy-ros-gz ros-jazzy-ros-gz-sim \
    qtwayland5 \
 && rm -rf /var/lib/apt/lists/*

# Default perf-friendly env
ENV NVIDIA_DRIVER_CAPABILITIES=all \
    QT_QPA_PLATFORM=wayland \
    GZ_RENDERING_ENGINE=ogre2 \
    GZ_GUI_FPS=20 \
    vblank_mode=0

# ROS env
RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc

WORKDIR /ws
