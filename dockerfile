# Dockerfile
FROM osrf/ros:jazzy-desktop

RUN apt-get update && apt-get install -y \
    git curl vim bash-completion build-essential cmake \
    python3-pip python3-colcon-common-extensions mesa-utils \
 && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    ros-jazzy-moveit ros-jazzy-moveit-resources-panda-moveit-config \
 && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    ros-jazzy-ros-gz ros-jazzy-ros-gz-sim \
 && rm -rf /var/lib/apt/lists/*

RUN echo "source /opt/ros/jazzy/setup.bash" >> /etc/bash.bashrc

WORKDIR /ws
