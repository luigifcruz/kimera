#include "kimera/state.hpp"
#include "kimera/transport.hpp"
#include "kimera/loopback.hpp"
#include "kimera/codec.hpp"
#include "kimera/client.hpp"
#include "kimera/render.hpp"

using namespace Kimera;

void receiver(State& state, Client& cli) {
    Socket socket(state);
    Loopback loopback(state);
    Decoder decoder(state);
    Resampler resampler(state, state.out_format);
    Render render(state);

    if (!socket.LoadClient()) return;
    if (CHECK(state.sink, Interfaces::LOOPBACK))
        if (!loopback.LoadSink()) return;

    if (CHECK(state.pipe, Interfaces::GPU_RESAMPLE) ||
        CHECK(state.pipe, Interfaces::FILTER) ||
        CHECK(state.sink, Interfaces::DISPLAY)) {
        if (!render.LoadInput(state.in_format)) return;
    }

    if (CHECK(state.pipe, Interfaces::FILTER)) {
        if (!render.LoadFilter()) return;
    }

    if (CHECK(state.sink, Interfaces::DISPLAY)) {
        if (!render.LoadDisplay()) return;
    }

    if (CHECK(state.pipe, Interfaces::GPU_RESAMPLE) ||
        CHECK(state.pipe, Interfaces::FILTER)) {
        if (!render.LoadOutput(state.out_format)) return;
    }

    render.CommitPipeline();

    Client::PrintState(state);
    render.PrintMeta();

    AVPacket* packet = NULL;
    while ((packet = socket.Pull()) && !cli.ShouldStop()) {
        if (decoder.Push(packet)) {
            AVFrame* frame = decoder.Pull();

            if (!render.Push(frame)) break;
            if (!render.Filter()) break;
            if (!render.Draw()) break;
            if (!(frame = decoder.Pull())) break;

            if (!resampler.Push(frame)) break;
            if (CHECK(state.sink, Interfaces::LOOPBACK))
                if (!loopback.Push(resampler.Pull())) break;
        }
    }
}

void transmitter(State& state, Client& cli) {
    AVFrame* frame;
    Socket socket(state);
    Loopback loopback(state);
    Encoder encoder(state);
    Resampler resampler(state, state.out_format);
    Render render(state);

    if (!socket.LoadServer()) return;
    if (!loopback.LoadSource()) return;

    if (CHECK(state.pipe, Interfaces::GPU_RESAMPLE) ||
        CHECK(state.pipe, Interfaces::FILTER) ||
        CHECK(state.sink, Interfaces::DISPLAY)) {
        if (!render.LoadInput(state.in_format)) return;
    }

    if (CHECK(state.pipe, Interfaces::FILTER)) {
        if (!render.LoadFilter()) return;
    }

    if (CHECK(state.sink, Interfaces::DISPLAY)) {
        if (!render.LoadDisplay()) return;
    }

    if (CHECK(state.pipe, Interfaces::GPU_RESAMPLE) ||
        CHECK(state.pipe, Interfaces::FILTER)) {
        if (!render.LoadOutput(state.out_format)) return;
    }

    render.CommitPipeline();

    Client::PrintState(state);
    render.PrintMeta();

    while (!cli.ShouldStop()) {
        if ((frame = loopback.Pull())) {
            if (!render.Push(frame)) break;
            if (!render.Filter()) break;
            if (!render.Draw()) break;
            if (!(frame = render.Pull())) break;

            if (!resampler.Push(frame)) break;
            if (encoder.Push(resampler.Pull()))
                if(!socket.Push(encoder.Pull())) break;
        }
    }
}

int main(int argc, char *argv[]) {
    Kimera::State state;
    Kimera::Client client(state);
    return client.Attach(argc, argv, transmitter, receiver);
}
