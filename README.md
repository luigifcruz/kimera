# 🐙 Kimera (Work-in-Progress)
### Low-latency hardware accelerated codec based video streaming utility.

## Features
- ⚡ Low-latency (~35ms).
- 🦑 Multi-Platform (Linux, macOS, Windows).
- 🔋 Low-overhead with Hardware Acceleration.
- 🖼️ Works with any codec (HEVC, AVC, VP9, VP8, AV1).
- 🧳 Supports multiple transport streams (UDP, TCP, UNIX, GNURadio).

<p align="center">
<img src="https://github.com/luigifreitas/kimera/raw/master/assets/kimera_macos.png" />
</p>

## Compatibility
### Interfacing
|    OS    |  Display  |   Screen-Capture   | Native Video Source | Native Video Sink |
|----------|-----------|--------------------|---------------------|-------------------|
| Linux    | ✅ (GLFW) | ✅ (XCB)          | ✅ (V4L2)          | ✅ (V4L2)         |
| macOS    | ✅ (GLFW) | ✅ (AVFoundation) | ✅ (AVFoundation)  | ❌                |
| Windows  | ✅ (GLFW) | ❌                | ❌                 | ❌                |

### Hardware Acceleration
|  Technology  | Encode | Decode |         Description         |
|--------------|--------|--------|-----------------------------|
| VideoToolBox | ✅     | ✅     | Apple Devices (macOS & iOS) |
| NVENC/CUVID  | ✅     | ✅     | Modern Nvidia GPUs          |
| OMX          | ✅     | ✅     | Broadcom SoC (Raspberry Pi) |
| VAAPI        | ❌     | ❌     | Modern Intel CPUs           |

## Installation 
Pre-compiled binary packages will be available once this app reaches beta. For now, if you want to try the pre-release version of the app, you should compile it yourself by following the instructions below.

### Dependencies
- GLFW
- OpenGL ES >2.0 or OpenGL >3.3
- LibAV >12.3
- CMake >3.13
- XCB (For Linux)
- Xcode Toolchain (For macOS)

#### Linux (Debian Based)
```shell
$ sudo apt install libavcodec-dev libavfilter-dev libavutil-dev libavformat-dev libglfw3-dev libyaml-dev libssl-deb build-essentials cmake git
```

#### macOS
```shell
$ brew install libyaml glfw3 libav cmake
```

### Compilation
```shell
$ git clone https://github.com/luigifreitas/kimera.git
$ cd kimera
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j
$ sudo make install
```

## Usage
Create a server with the `/dev/video0` as input and localhost TCP as output.
```shell
$ ./tunnel tx ../examples/linux.yml
```

Receive the video from the TCP server and display it on the screen.
```shell
$ ./tunnel rx ../examples/linux.yml
```

## License
This module is distributed under a [GPL-2.0 License](https://raw.githubusercontent.com/luigifreitas/kimera/master/LICENSE).

## Roadmap
This is a list of unfinished tasks that I pretend to pursue soon. Pull requests are more than welcome!
- [ ] Implement AVFoundation Native Sink Device.
- [ ] Add SSL support for secure connections.
- [ ] Write Windows Support.
- [ ] Write Android Support.
- [ ] Write iOS Support.
