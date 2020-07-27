import sys
import kimera

def StartTransmiter(state):
    socket = kimera.Socket(state)
    loopback = kimera.Loopback(state)
    encoder = kimera.Encoder(state)
    resampler = kimera.Resampler(state)

    if not socket.LoadServer():
        return

    if not loopback.LoadSource():
        return

    while (True):
        resampler.Push(loopback.Pull())
        if encoder.Push(resampler.Pull()):
            socket.Push(encoder.Pull())

def StartReceiver(state):
    pass

if __name__ == "__main__":
    state = kimera.State()

    if sys.argv[1] == "tx":
        state.LoadTransmitter()
    if sys.argv[1] == "rx":
        state.LoadReceiver()

    state.ParseConfigFile(sys.argv[2])

    kimera.Client.PrintState(state)

    if sys.argv[1] == "tx":
        StartTransmiter(state)
    if sys.argv[1] == "rx":
        StartReceiver(state)

