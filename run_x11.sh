xhost +local:root

docker run --rm -it --name ros-gui-x11 \
  --user $(id -u):$(id -g) \
  --gpus all --device=/dev/dxg \
  --ipc=host --shm-size=2g \
  -e DISPLAY \
  -e QT_QPA_PLATFORM=xcb -e QT_X11_NO_MITSHM=1 \
  -e GZ_RENDERING_ENGINE=ogre2 -e GZ_GUI_FPS=20 -e vblank_mode=0 \
  -e LIBGL_DRIVERS_PATH=/usr/lib/wsl/lib/dri \
  -e LD_LIBRARY_PATH=/usr/lib/wsl/lib:$LD_LIBRARY_PATH \
  -e MESA_D3D12_DEFAULT_ADAPTER_NAME=NVIDIA -e MESA_NO_ERROR=1 \
  -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
  -v /usr/lib/wsl:/usr/lib/wsl:ro \
  -v /usr/share/vulkan/icd.d:/usr/share/vulkan/icd.d:ro \
  -v /usr/share/vulkan/implicit_layer.d:/usr/share/vulkan/implicit_layer.d:ro \
  -v /usr/share/glvnd/egl_vendor.d:/usr/share/glvnd/egl_vendor.d:ro \
  ros-jazzy-dev bash