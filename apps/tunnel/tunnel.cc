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
//  RenderState* render = init_render();

    if (!loopback.LoadSink()) return;
    if (!socket.LoadClient()) return;
//  ok &= open_render(render, state);

    Client::PrintState(state);
//  render_print_meta(render);

    AVPacket* packet = NULL;
    while (!(packet = socket.Pull()) && !cli.ShouldStop()) {
        if (decoder.Push(packet)) {
            AVFrame* frame = decoder.Pull();

/*
            Interfaces pipe = state->pipe;
            Interfaces sink = state->sink;

            if (pipe & GPU_RESAMPLE || pipe & FILTER || sink & DISPLAY)
                if (!render_push_frame(render, frame)) break;
            if (pipe & FILTER)
                if (!render_proc_frame(render)) break;
            if (sink & DISPLAY)
                if (!render_draw_frame(render)) break;
            if (pipe & GPU_RESAMPLE || pipe & FILTER) {
                if (!render_pull_frame(render)) break;
                frame = render->frame;
            }
*/

            if (!resampler.Push(frame)) break;

            if (CHECK(state.sink, Interfaces::LOOPBACK))
                if (!loopback.Push(resampler.Pull())) break;
        }
    }
}

void transmitter(State& state, Client& cli) {
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

    while (cli.ShouldStop()) {
        AVFrame* frame = loopback.Pull();
        if (!frame) break;

        if (!render.Push(frame)) break;
        if (!render.Filter()) break;
        if (!render.Draw()) break;
        if (!(frame = render.Pull())) break;

        if (!resampler.Push(frame)) break;
        if (encoder.Push(resampler.Pull()))
            if(!socket.Push(encoder.Pull())) break;
    }
}

int main(int argc, char *argv[]) {
    Kimera::State state;
    Kimera::Client client(state);
    return client.Attach(argc, argv, transmitter, receiver);
}
