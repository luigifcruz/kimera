ffmpeg:
    ./client test_socket | ffmpeg -analyzeduration 0 -fflags -nobuffer
    -probesize 32 -filter_complex format=yuv420p -framerate 30 -i - -f v4l2 /dev/video0

ffplay:
    ./client test_socket | ffplay -analyzeduration 0 -fflags -nobuffer -probesize 32 -framerate 30 -i -
