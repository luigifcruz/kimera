#include "kimera/client.hpp"

namespace Kimera {

void Client::ParseHeader(const YAML::Node& config) {
    if (config["width"])
        state.width = config["width"].as<int>();
    if (config["height"])
        state.height = config["height"].as<int>();
    if (config["framerate"])
        state.framerate = config["framerate"].as<int>();
    if (config["bitrate"])
        state.bitrate = config["bitrate"].as<int>();
    if (config["packet_size"])
        state.packet_size = config["packet_size"].as<int>();
}

void Client::ParseInterfaces(const YAML::Node& config, Interfaces& out) {
    for (const auto& it : config) {
        std::string src_str = it.as<std::string>();
        auto interface = magic_enum::enum_cast<Interfaces>(src_str);
        out = interface.value() | out;
    }
}

void Client::ParseBody(const YAML::Node& config) {
    if (config["source"])
        ParseInterfaces(config["source"], state.source);
    if (config["pipe"])
        ParseInterfaces(config["pipe"], state.pipe);
    if (config["sink"])
        ParseInterfaces(config["sink"], state.sink);

    if (config["codec"])
        state.coder_name = config["codec"].as<std::string>();
    if (config["address"])
        state.address = config["address"].as<std::string>();
    if (config["device"])
        state.loopback = config["device"].as<std::string>();
    if (config["port"])
        state.port = config["port"].as<int>();
}

bool Client::ParseConfigFile(char* path) {
    try {
        YAML::Node config = YAML::LoadFile(path);

        ParseHeader(config);

        switch (state.mode) {
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