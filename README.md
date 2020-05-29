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
| Linux    | ✅ (SDL)  | ✅ (XCB)           | ✅ (V4L2)           | ✅ (V4L2)         |
| macOS    | ✅ (SDL)  | ✅ (AVFoundation)  | ✅ (AVFoundation)   | ❌                |
| Windows  | ❌        | ❌                 | ❌                  | ❌                |

### Hardware Acceleration
|  Technology  | Encode | Decode |         Description         |
|--------------|--------|--------|-----------------------------|
| VideoToolBox | ✅     | ✅     | Apple Devices (macOS & iOS) |
| NVENC/CUVID  | ✅     | ✅     | Modern Nvidia GPUs          |
| OMX          | ✅     | ✅     | Broadcom SoC (Raspberry Pi) |
| VAAPI        | ❌     | ❌     | Intel Modern CPUs           |

## Installation 
Pre-compiled binary packages will be available once this app reaches beta. For now, if you want to try the pre-release version of the app, you should compile it yourself by following the instructions below.

### Dependencies
- SDL2
- SDL TTF
- LibAV >12.3
- CMake >3.13
- XCB (For Linux)
- Xcode Toolchain (For macOS)

#### Linux (Debian Based)
```shell
$ sudo apt install libavcodec-dev libavfilter-dev libavutil-dev libavformat-dev libsdl2-dev libsdl2-ttf-dev libyaml-dev
```

#### macOS
```shell
$ brew install libyaml sdl2 sdl2_ttf libav cmake
```

### Compilation
```shell
$ git clone https://github.com/luigifreitas/kimera.git
$ cd kimera
$ mkdir build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make -j
$ sudo make install
```

## Usage
Create a server with the `/dev/video0` as input and localhost TCP as output.
```shell
$ ./kimera tx ../examples/linux.yml
```

Receive the video from the TCP server and display it on the screen.
```shell
$ ./kimera rx ../examples/linux.yml
```

## License
This module is distributed under a [GPL-2.0 License](https://raw.githubusercontent.com/luigifreitas/kimera/master/LICENSE).

## Roadmap
This is a list of unfinished tasks that I pretend to pursue soon. Pull requests are more than welcome!
- [ ] Implement AVFoundation Native Sink Device.
- [ ] Write Windows Support.
- [ ] Write Android Support.
- [ ] Write iOS Support.
