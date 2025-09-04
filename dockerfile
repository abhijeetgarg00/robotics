# Dockerfile
FROM osrf/ros:jazzy-desktop

# Basic tools
RUN apt-get update && apt-get install -y \
    git curl vim bash-completion build-essential cmake \
    python3-pip python3-colcon-common-extensions \
    mesa-utils \
    && rm -rf /var/lib/apt/lists/*

# MoveIt 2 + Panda sample config
RUN apt-get update && apt-get install -y \
    ros-jazzy-moveit \
    ros-jazzy-moveit-resources-panda-moveit-config \
    && rm -rf /var/lib/apt/lists/*

# Gazebo Harmonic (via ros-gz) + bridge
RUN apt-get update && apt-get install -y \
    ros-jazzy-ros-gz \
    && rm -rf /var/lib/apt/lists/*

# QoL: source ROS automatically for all shells
RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc

RUN mkdir -p ~/ros2

WORKDIR ~/ros2
