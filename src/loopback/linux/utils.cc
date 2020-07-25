#include "kimera/loopback/linux.hpp"

namespace Kimera {

unsigned int ff_to_v4l(enum AVPixelFormat input) {
    switch (input) {
    case AV_PIX_FMT_RGB24:          return V4L2_PIX_FMT_RGB24;
    case AV_PIX_FMT_YUYV422:        return V4L2_PIX_FMT_YUYV;
    case AV_PIX_FMT_YUV420P:        return V4L2_PIX_FMT_YUV420;
    case AV_PIX_FMT_YUV422P:        return V4L2_PIX_FMT_YUV422P;
    case AV_PIX_FMT_UYVY422:        return V4L2_PIX_FMT_UYVY;
    case AV_PIX_FMT_YUV411P:        return V4L2_PIX_FMT_YUV411P;
    case AV_PIX_FMT_YUV410P:        return V4L2_PIX_FMT_YUV410;
    case AV_PIX_FMT_RGB555LE:       return V4L2_PIX_FMT_RGB555;
    case AV_PIX_FMT_RGB555BE:       return V4L2_PIX_FMT_RGB555X;
    case AV_PIX_FMT_RGB565LE:       return V4L2_PIX_FMT_RGB565;
    case AV_PIX_FMT_RGB565BE:       return V4L2_PIX_FMT_RGB565X;
    case AV_PIX_FMT_BGR24:          return V4L2_PIX_FMT_BGR24;
    case AV_PIX_FMT_GRAY8:          return V4L2_PIX_FMT_GREY;
    case AV_PIX_FMT_NV12:           return V4L2_PIX_FMT_NV12;
#ifdef V4L2_PIX_FMT_SRGGB8
    case AV_PIX_FMT_BAYER_BGGR8:    return V4L2_PIX_FMT_SBGGR8;
    case AV_PIX_FMT_BAYER_GBRG8:    return V4L2_PIX_FMT_SGBRG8;
    case AV_PIX_FMT_BAYER_GRBG8:    return V4L2_PIX_FMT_SGRBG8;
    case AV_PIX_FMT_BAYER_RGGB8:    return V4L2_PIX_FMT_SRGGB8;
#endif
#ifdef V4L2_PIX_FMT_XBGR32
    case AV_PIX_FMT_BGR0:           return V4L2_PIX_FMT_XBGR32;
    case AV_PIX_FMT_0RGB:           return V4L2_PIX_FMT_XRGB32;
    case AV_PIX_FMT_BGRA:           return V4L2_PIX_FMT_ABGR32;
    case AV_PIX_FMT_ARGB:           return V4L2_PIX_FMT_ARGB32;
#elif
    case AV_PIX_FMT_BGR0:           return V4L2_PIX_FMT_BGR32;
    case AV_PIX_FMT_0RGB:           return V4L2_PIX_FMT_RGB32;
#endif
    default:
        printf("[LOOPBACK] Selected pixel format (%d) not supported.\n", input);
        return -1;
    }
}

enum AVPixelFormat v4l_to_ff(unsigned int input) {
    switch (input) {
    case V4L2_PIX_FMT_RGB24:        return AV_PIX_FMT_RGB24;
    case V4L2_PIX_FMT_YUYV:         return AV_PIX_FMT_YUYV422;
    case V4L2_PIX_FMT_YUV420:       return AV_PIX_FMT_YUV420P;
    case V4L2_PIX_FMT_YUV422P:      return AV_PIX_FMT_YUV422P;
    case V4L2_PIX_FMT_UYVY:         return AV_PIX_FMT_UYVY422;
    case V4L2_PIX_FMT_YUV411P:      return AV_PIX_FMT_YUV411P;
    case V4L2_PIX_FMT_YUV410:       return AV_PIX_FMT_YUV410P;
    case V4L2_PIX_FMT_RGB555:       return AV_PIX_FMT_RGB555LE;
    case V4L2_PIX_FMT_RGB555X:      return AV_PIX_FMT_RGB555BE;
    case V4L2_PIX_FMT_RGB565:       return AV_PIX_FMT_RGB565LE;
    case V4L2_PIX_FMT_RGB565X:      return AV_PIX_FMT_RGB565BE;
    case V4L2_PIX_FMT_BGR24:        return AV_PIX_FMT_BGR24;
    case V4L2_PIX_FMT_BGR32:        return AV_PIX_FMT_BGR0;
    case V4L2_PIX_FMT_RGB32:        return AV_PIX_FMT_0RGB;
    case V4L2_PIX_FMT_GREY:         return AV_PIX_FMT_GRAY8;
    case V4L2_PIX_FMT_NV12:         return AV_PIX_FMT_NV12;
#ifdef V4L2_PIX_FMT_SRGGB8
    case V4L2_PIX_FMT_SBGGR8:       return AV_PIX_FMT_BAYER_BGGR8;
    case V4L2_PIX_FMT_SGBRG8:       return AV_PIX_FMT_BAYER_GBRG8;
    case V4L2_PIX_FMT_SGRBG8:       return AV_PIX_FMT_BAYER_GRBG8;
    case V4L2_PIX_FMT_SRGGB8:       return AV_PIX_FMT_BAYER_RGGB8;
#endif
#ifdef V4L2_PIX_FMT_XBGR32
    case V4L2_PIX_FMT_XBGR32:       return AV_PIX_FMT_BGR0;
    case V4L2_PIX_FMT_XRGB32:       return AV_PIX_FMT_0RGB;
    case V4L2_PIX_FMT_ABGR32:       return AV_PIX_FMT_BGRA;
    case V4L2_PIX_FMT_ARGB32:       return AV_PIX_FMT_ARGB;
#endif
#ifdef V4L2_PIX_FMT_Z16
    case V4L2_PIX_FMT_Z16:          return AV_PIX_FMT_GRAY16LE;
#endif
#ifdef V4L2_PIX_FMT_Y16
    case V4L2_PIX_FMT_Y16:          return AV_PIX_FMT_GRAY16LE;
#endif
    default:
        printf("[LOOPBACK] Selected pixel format (%d) not supported.\n", input);
        return AV_PIX_FMT_NONE;
    }
}

unsigned int find_v4l_format(int fd, unsigned int preferred) {
    struct v4l2_fmtdesc fmtdesc;
    unsigned int opt = 0;
    memset(&fmtdesc, 0, sizeof(fmtdesc));
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
        printf("[LOOPBACK] Device Format: %s\n", fmtdesc.description);
        opt = fmtdesc.pixelformat;
        if (fmtdesc.pixelformat == preferred)
            return fmtdesc.pixelformat;
        fmtdesc.index++;
    }
    return opt;
}

} // namespace Kimera