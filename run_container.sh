docker run --rm -it --name ros-gui \
  --user $(id -u):$(id -g) \
  -e DISPLAY -e QT_X11_NO_MITSHM=1 \
  -e XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir \
  -e PULSE_SERVER=unix:/mnt/wslg/PulseServer \
  -e GZ_RENDERING_ENGINE=ogre2 -e GZ_GUI_FPS=30 \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v /mnt/wslg:/mnt/wslg \
  --device=/dev/dxg --gpus all \
  ros-jazzy-dev bash