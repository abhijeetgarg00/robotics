docker run --rm -it --name ros-sim \
  --gpus all --device=/dev/dxg \
  --network host --ipc=host --shm-size=2g \
  ros-jazzy-dev bash