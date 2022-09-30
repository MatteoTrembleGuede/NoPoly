#ifndef ENCODER_H
#define ENCODER_H
#include <cassert>

extern "C" {
#include <FFmpeg/libswscale/swscale.h>
#include <FFmpeg/libavcodec/avcodec.h>
#include <FFmpeg/libavformat/avformat.h>
#include <FFmpeg/libavutil/opt.h>
#include <FFmpeg/libavutil/hwcontext.h>
#include <FFmpeg/libavfilter/avfilter.h>
}

#include <string>
#include "Texture.h"

class Encoder
{
public:
    Encoder(std::string fileName, int width, int height, int frameRate, Texture*& debug);
    ~Encoder();
    void addFrame(unsigned char* pixels);
    void flush();

private:
    void encodeFrame(AVFrame* frame);

    // members
    int m_frameId = 1;
    Texture*& debug;

    SwsContext* swsCtx;
    AVCodecContext* m_encoder = nullptr;
    AVFormatContext* m_muxer = nullptr;
    AVStream* m_avStream = nullptr;
    AVBufferRef* m_device = nullptr;
};

#endif // ENCODER_H