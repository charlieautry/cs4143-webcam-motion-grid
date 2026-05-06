# Webcam to 3D Motion Grid

CS 4143 final project. Takes a live webcam feed and turns it into a 3D grid of extruded columns. Each column gets its color from the matching pixel and its height from motion detected with dense optical flow. You can orbit around with the mouse.

C++ / OpenGL 3.3 / GLFW / GLM / OpenCV. Built with CMake on Windows + MSVC.

## Building

You need Visual Studio (2019 or newer), CMake, and OpenCV 4 installed somewhere CMake can find it. From the `build/` folder:

```
cmake -G "Visual Studio 17 2022" ..
cmake --build . --config Release
```

If it can't find OpenCV, point it at your install:

```
cmake -DOpenCV_DIR="C:/path/to/opencv/build" -G "Visual Studio 17 2022" ..
```

GLFW gets built as part of the project so there's nothing to install for that.

## Running

The exe lands in `bin/`. You have to run it from there so the shader paths resolve:

```
cd bin
final_project.exe
```

Default webcam (device 0) is required.

## Controls

Mouse: left-drag to orbit, right-drag to pan, scroll to zoom.

Keys:
- R reset the camera, F snap to a 45 degree view of the grid
- C toggle color/mono, M toggle mirror, H toggle the HUD
- Up/Down change max height, Left/Right change motion sensitivity
- [ and ] cycle through grid resolutions (32x24 up to 128x96)
- Esc quits

## How it works

OpenCV grabs a frame, downsamples it to grid resolution for color, and runs Farneback optical flow at 320x240 for motion. The motion magnitudes get downsampled and fed to the grid, which lerps each column's height (fast rise, slow fall — looked too snappy without it). Heights and colors go to the GPU as instanced attributes so the whole grid is one draw call. Phong shading with light intensity tied to average motion across the scene, so things get brighter when you move.