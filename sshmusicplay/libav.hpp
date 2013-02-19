#ifndef LIBAV_HPP
#define LIBAV_HPP

// Header for loading libav stuff

extern "C"
{
#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#endif // LIBAV_HPP
