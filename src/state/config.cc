#include "kimera/state.hpp"
#include <libavutil/pixfmt.h>

namespace Kimera {

void State::ParseHeader(const YAML::Node& config) {
    if (config["width"])
        width = config["width"].as<int>();
    if (config["height"])
        height = config["height"].as<int>();
    if (config["framerate"])
        framerate = config["framerate"].as<int>();
    if (config["bitrate"])
        bitrate = config["bitrate"].as<int>();
    if (config["packet_size"])
        packet_size = config["packet_size"].as<int>();
}

void State::ParseInterfaces(const YAML::Node& config, Interfaces& out) {
    for (const auto& it : config) {
        std::string src_str = it.as<std::string>();
        auto interface = magic_enum::enum_cast<Interfaces>(src_str);
        if (interface.has_value())
            out = interface.value() | out;
    }
}

void State::ParsePixelFormat(const YAML::Node& config, enum AVPixelFormat& fmt) {
    auto pfmt_str = config.as<std::string>();
    auto pfmt = magic_enum::enum_cast<AVPixelFormat>(pfmt_str);
    if (pfmt.has_value())
        fmt = pfmt.value();
}

void State::ParseBody(const YAML::Node& config) {
    if (config["source"])
        ParseInterfaces(config["source"], source);
    if (config["pipe"])
        ParseInterfaces(config["pipe"], pipe);
    if (config["sink"])
        ParseInterfaces(config["sink"], sink);

    if (config["in_format"])
        ParsePixelFormat(config["in_format"], in_format);
    if (config["out_format"])
        ParsePixelFormat(config["out_format"], out_format);

    if (config["codec"])
        coder_name = config["codec"].as<std::string>();
    if (config["address"])
        address = config["address"].as<std::string>();
    if (config["device"])
        loopback = config["device"].as<std::string>();
    if (config["port"])
        port = config["port"].as<int>();
}

bool State::ParseConfigFile(char* path) {
    try {
        YAML::Node config = YAML::LoadFile(path);

        ParseHeader(config);

        switch (mode) {
        case Mode::TRANSMITTER:
            ParseBody(config["transmitter"]);
            break;
        case Mode::RECEIVER:
            ParseBody(config["receiver"]);
            break;
        default: break;
        }
    } catch(YAML::Exception& e) {
        std::cout << "[CLIENT] Can't parse file: " << e.what() << "\n";
        return false;
    }

    return true;
}

} // namespace Kimera

