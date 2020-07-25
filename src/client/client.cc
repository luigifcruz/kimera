#include "kimera/client.hpp"
#include "kimera/state.hpp"

volatile sig_atomic_t stop_handler;

void InterruptHandler(int signum) {
    if (stop_handler == 1) {
      exit(-1);
    }
    printf("Safely exiting, press Crtl-C again to force shutdown.\n");
    stop_handler = 1;
}

Client::Client(State& state) : state(state) {
    this->stop = &stop_handler;
}

void Client::PrintVersion() {
    printf("Kimera Version: %d.%d\n", KIMERA_VERSION_MAJOR, KIMERA_VERSION_MINOR);
    printf("Ffmpeg Version: %s\n", av_version_info());
}

void Client::PrintHelp() {
    printf("Usage:\n   kimera [mode] [profile] [config]\n");
    printf("    - [profile] (required)  Name of the selected profile from the configuration file.\n");
    printf("    - [mode]    (required)  Operation Mode: Transmitter (tx) or Receiver (rx).\n");
    printf("    - [config]  (optional)  Path of the configuration file containing profiles.\n");
    printf("Available Flags:\n    -h Print Help\n    -v Print Version\n    -k Print Crypto Key\n");
    printf("Example:\n   kimera tx rpi4_picam_v1 profiles.yml\n");
}

void Client::PrintInterface(Interfaces interfaces) {
    if (interfaces == Interfaces::NONE)
        printf(" NONE");
    if (CHECK(interfaces, Interfaces::TCP))
        printf(" TCP");
    if (CHECK(interfaces, Interfaces::UDP))
        printf(" UDP");
    if (CHECK(interfaces, Interfaces::UNIX))
        printf(" UNIX");
    if (CHECK(interfaces, Interfaces::DISPLAY))
        printf(" DISPLAY");
    if (CHECK(interfaces, Interfaces::LOOPBACK))
        printf(" LOOPBACK");
    if (CHECK(interfaces, Interfaces::FILTER))
        printf(" FILTER");
    if (CHECK(interfaces, Interfaces::RESAMPLE))
        printf(" RESAMPLE");
    if (CHECK(interfaces, Interfaces::GPU_RESAMPLE))
        printf(" GPU_RESAMPLE");
    if (CHECK(interfaces, Interfaces::CRYPTO))
        printf(" CRYPTO");
    printf("\n");
}

void Client::PrintState() {
    printf(".   CURRENT STATE\n");
    printf("├── Dimensions: %dx%d\n", state.width, state.height);
    printf("├── Framerate:  %d FPS\n", state.framerate);
    printf("├── Bitrate:    %d bps\n", state.bitrate);
    printf("└── Packet Len: %d Bytes\n", state.packet_size);

    if (state.mode == Mode::NONE) {
        printf("    .   MODE UNKNOW\n");
        return;
    }

    if (state.mode == Mode::TRANSMITTER)
        printf("    .   TRANSMITTER\n");

    if (state.mode == Mode::RECEIVER)
        printf("    .   RECEIVER\n");

    printf("    ├── Source: ");
    PrintInterface(state.source);
    printf("    ├── Pipe:   ");
    PrintInterface(state.pipe);
    printf("    ├── Sink:   ");
    PrintInterface(state.sink);

    printf("    ├── Device:  %s\n", state.loopback.c_str());
    printf("    ├── Address: %s\n", state.address.c_str());
    printf("    ├── Port:    %d\n", state.port);
    printf("    └── Codec:   %s\n", state.coder_name.c_str());
}

void Client::PrintKey() {
    char b64_key[MAX_KEY_LEN];
    char bin_key[DEFAULT_KEY_LEN];
    if (!Crypto::NewKey(bin_key, DEFAULT_KEY_LEN)) return;
    Crypto::Bytes2Base(bin_key, DEFAULT_KEY_LEN, (char*)&b64_key);
    printf("%s\n", b64_key);
}

int Client::Attach(int argc, char *argv[], void(*tx)(State&, Client&), void(*rx)(State&, Client&)) {
    // Set Windows console to UTF-8
#ifdef KIMERA_WINDOWS
    system("chcp 65001");
#endif

    // Register signal handler.
    signal(SIGINT, InterruptHandler);

    // Parse Command-Line Arguments
    if (argc < 2) {
        printf("Not enough arguments.\n");
        PrintHelp();
        return -1;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
        PrintHelp();
        return 0;
    }

    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
        PrintVersion();
        return 0;
    }

    if (!strcmp(argv[1], "-k") || !strcmp(argv[1], "--key")) {
        PrintKey();
        return 0;
    }

    if (argc < 3) {
        printf("Not enough arguments.\n");
        PrintHelp();
        return -1;
    }

    // Declare Default Settings
    char* config_path = argv[2];

    switch (argv[1][0]) {
        case 't':
        case 'T':
            state.mode = Mode::TRANSMITTER;
            break;
        case 'r':
        case 'R':
            state.mode = Mode::RECEIVER;
            break;
        default:
            printf("Mode (%s) not valid.\n", argv[1]);
            PrintHelp();
            return -1;
    }

    if (!ParseConfigFile(config_path)) return -1;

    if (CHECK(state.pipe, Interfaces::CRYPTO) && state.crypto_key.size() < 8) {
        if (CHECK(state.mode, Mode::TRANSMITTER)) {
            char bin_key[DEFAULT_KEY_LEN];
            if (!Crypto::NewKey(bin_key, DEFAULT_KEY_LEN)) return -1;
            Crypto::Bytes2Base((char*)&bin_key, DEFAULT_KEY_LEN, (char*)state.crypto_key.c_str());

            printf("\nNo pre-shared key found, generating one...\n");
            printf("TIP: This can be pre-defined in the configuration file as `crypto_key`.\n\n");
            printf("%s\n\n", state.crypto_key.c_str());
        }

        if (CHECK(state.mode, Mode::RECEIVER)) {
            printf("\nNo pre-shared key found! Type the Transmitter Pre-shared Key:\n");
            printf("TIP: This can be pre-defined in the configuration file as `crypto_key`.\n\n");
            printf("KEY> ");

            if (state.crypto_key.empty()) {
                printf("[CRYPTO] User entered an invalid pre-shared key. Exiting...\n");
                throw;
            }

            printf("\n");
        }
    }

    switch (state.mode) {
        case Mode::TRANSMITTER:
            (void)(*tx)(state, *this);
            break;
        case Mode::RECEIVER:
            (void)(*rx)(state, *this);
            break;
        default: break;
    }

    return 0;
}

bool Client::ShouldStop() {
    return this->stop;
}
