package com.example.camera;

import androidx.appcompat.app.AppCompatActivity;
import android.Manifest;
import android.content.pm.PackageManager;
import android.hardware.camera2.*;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.net.LocalServerSocket;
import android.net.LocalSocket;
import android.os.Bundle;
import android.system.ErrnoException;
import android.system.Os;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.TextView;

import androidx.core.app.ActivityCompat;

import java.io.FileDescriptor;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
    protected static final String TAG = "VideoProcessing";
    private static final int MY_CAMERA_REQUEST_CODE = 100;
    private static final int NO_PTS = -1;

    private String cameraId = null;
    private CameraDevice cameraDevice = null;
    private LocalServerSocket serverSocket = null;
    private final ByteBuffer headerBuffer = ByteBuffer.allocate(12);
    private long ptsOrigin;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);

        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    serverSocket = new LocalServerSocket("com.example.camera.video_stream");
                } catch (IOException e) {
                    e.printStackTrace();
                }

                while (true) {
                    try {
                        final LocalSocket connection = serverSocket.accept();
                        runOnUiThread(new Runnable() {
                            public void run() {
                                try {
                                    openCamera(connection);
                                } catch (CameraAccessException e) {
                                    e.printStackTrace();
                                }
                            }
                        });
                    } catch (IOException e) {
                        Log.e(TAG, "Socket/Encoder error.");
                    }
                }
            }
        }).start();
    }

    //
    // Camera Methods
    //

    public void openCamera(final LocalSocket connection) throws CameraAccessException {
        CameraManager manager = (CameraManager) getSystemService(CAMERA_SERVICE);

        for (String id : manager.getCameraIdList()) {
            CameraCharacteristics c = manager.getCameraCharacteristics(id);
            Integer facing = c.get(CameraCharacteristics.LENS_FACING);
            if (facing != null && facing.equals(CameraCharacteristics.LENS_FACING_BACK)) {
                cameraId = id;
                break;
            }
        }

        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG, "Not authorized.");
            requestPermissions(new String[]{Manifest.permission.CAMERA}, MY_CAMERA_REQUEST_CODE);
            return;
        }

        manager.openCamera(cameraId, new CameraDevice.StateCallback() {
            @Override
            public void onOpened(CameraDevice camera) {
                Log.i(TAG, "CameraDevice.StateCallback onOpened");
                cameraDevice = camera;
                makeDecoder(connection);
            }

            @Override
            public void onDisconnected(CameraDevice camera) {
                Log.w(TAG, "CameraDevice.StateCallback onDisconnected");
            }

            @Override
            public void onError(CameraDevice camera, int error) {
                Log.e(TAG, "CameraDevice.StateCallback onError " + error);
            }
        }, null);
    }

    //
    // Decoder Methods
    //

    private MediaCodec declareVideoDecoder(String videoCodecName) {
        try {
            return MediaCodec.createEncoderByType(videoCodecName);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    private MediaCodec declareAudioDecoder(String audioCodecName) {
        try {
            return MediaCodec.createEncoderByType(audioCodecName);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    private MediaFormat declareVideoFormat(String videoCodecName) {
        MediaFormat format = new MediaFormat();
        format.setString(MediaFormat.KEY_MIME, videoCodecName);
        format.setInteger(MediaFormat.KEY_BIT_RATE, 5000000);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        format.setLong(MediaFormat.KEY_REPEAT_PREVIOUS_FRAME_AFTER, 100_000);
        format.setInteger(MediaFormat.KEY_LATENCY, 0);
        format.setInteger(MediaFormat.KEY_PRIORITY, 0);
        format.setInteger(MediaFormat.KEY_WIDTH, 1920);
        format.setInteger(MediaFormat.KEY_HEIGHT, 1080);
        return format;
    }

    private MediaFormat declareAudioFormat(String audioCodecName) {
        MediaFormat format = new MediaFormat();
        format.setString(MediaFormat.KEY_MIME, audioCodecName);
        format.setInteger(MediaFormat.KEY_BIT_RATE, 5000000);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, 30);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        format.setLong(MediaFormat.KEY_REPEAT_PREVIOUS_FRAME_AFTER, 100_000);
        format.setInteger(MediaFormat.KEY_LATENCY, 0);
        format.setInteger(MediaFormat.KEY_PRIORITY, 0);
        format.setInteger(MediaFormat.KEY_WIDTH, 1920);
        format.setInteger(MediaFormat.KEY_HEIGHT, 1080);
        return format;
    }

    public void makeDecoder(final LocalSocket connection) {
        String videoCodecName = "video/hevc";
        String audioCodecName = "mp4a-latm";

        final MediaCodec videoCodec = declareVideoDecoder(videoCodecName);
        final MediaFormat videoFormat = declareVideoFormat(videoCodecName);
        videoCodec.configure(videoFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

        final MediaCodec audioCodec = declareVideoDecoder(videoCodecName);
        final MediaFormat audioFormat = declareVideoFormat(videoCodecName);
        audioCodec.configure(audioFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);

        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.cameraSurface);

        final Surface surface = surfaceView.getHolder().getSurface();
        final Surface codecSurface = videoCodec.createInputSurface();

        List<Surface> surfaces = new ArrayList<>();
        surfaces.add(codecSurface);
        surfaces.add(surface);
        videoCodec.start();
        encoderLoop(videoCodec, connection);

        try {
            cameraDevice.createCaptureSession(surfaces, new CameraCaptureSession.StateCallback() {

                @Override
                public void onConfigured(CameraCaptureSession session) {
                    Log.i(TAG, "CameraCaptureSession.StateCallback onConfigured");

                    try {
                        CaptureRequest.Builder builder = cameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
                        builder.addTarget(codecSurface);
                        builder.addTarget(surface);
                        session.setRepeatingRequest(builder.build(), null, null);

                    } catch (CameraAccessException e) {
                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession session) {
                    Log.e(TAG, "CameraDevice.createCaptureSession.onConfigureFailed");
                }

            }, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public void encoderLoop(final MediaCodec codec, final LocalSocket connection) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
                TextView textView = (TextView)findViewById(R.id.ptsLabel);

                while (connection.isConnected()) {
                    int outputBufferId = codec.dequeueOutputBuffer(bufferInfo, -1);

                    try {
                        if (outputBufferId >= 0) {
                            FileDescriptor fd = connection.getFileDescriptor();
                            ByteBuffer codecBuffer = codec.getOutputBuffer(outputBufferId);

                            headerBuffer.clear();

                            long pts;
                            if ((bufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
                                pts = NO_PTS;
                            } else {
                                if (ptsOrigin == 0) {
                                    ptsOrigin = bufferInfo.presentationTimeUs;
                                }
                                pts = bufferInfo.presentationTimeUs - ptsOrigin;
                            }
                            Log.e(TAG, String.valueOf(bufferInfo.size));
                            Log.e(TAG, String.valueOf(codecBuffer.remaining()));


                            //textView.setText(String.valueOf(pts));

                            headerBuffer.putLong(pts);
                            headerBuffer.putInt(codecBuffer.remaining());
                            headerBuffer.flip();

                            Os.write(fd, headerBuffer);
                            Os.write(fd, codecBuffer);
                        }
                    } catch (ErrnoException | IOException e) {
                        break;
                    } finally {
                        if (outputBufferId >= 0) {
                            codec.releaseOutputBuffer(outputBufferId, false);
                        }
                    }
                }

                cameraDevice.close();
            }
        }).start();
    }

}
