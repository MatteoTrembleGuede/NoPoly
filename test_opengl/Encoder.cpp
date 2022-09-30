#include "Encoder.h"
#include "UIResourceBrowser.h"

#define DEST_PIX_FMT AV_PIX_FMT_YUV420P
#define SRC_PIX_FMT AV_PIX_FMT_RGB24

Encoder::Encoder(std::string fileName, int width, int height, int frameRate, Texture*& _debug) : debug(_debug)
{
	assert(avformat_alloc_output_context2(&m_muxer, nullptr, nullptr, fileName.c_str()) == 0);
	assert(m_muxer != nullptr);

	const char* encoderName = UIResourceBrowser::GetExtension(fileName) == std::string("webm") ? "libvpx-vp9" : "libx264";
	//const char* encoderName = "libx264";
	const AVCodec* videoCodec = avcodec_find_encoder_by_name(encoderName);
	m_encoder = avcodec_alloc_context3(videoCodec);
	m_encoder->bit_rate = width * height * frameRate * 2;
	m_encoder->width = width;
	m_encoder->height = height;
	m_encoder->time_base = AVRational{ 1, frameRate };
	m_encoder->framerate = AVRational{ frameRate, 1 };

	m_encoder->gop_size = frameRate;
	m_encoder->max_b_frames = 1;
	m_encoder->pix_fmt = DEST_PIX_FMT;

	AVDictionary* opt = NULL;
	av_dict_set(&opt, "preset", "ultrafast", 0);
	av_dict_set(&opt, "crf", "0", 0);

	assert(avcodec_open2(m_encoder, videoCodec, &opt) == 0);
	av_dict_free(&opt);

	//assert(avcodec_open2(m_encoder, videoCodec, nullptr) == 0);

	m_muxer->video_codec_id = videoCodec->id;
	m_muxer->video_codec = videoCodec;

	m_avStream = avformat_new_stream(m_muxer, videoCodec);
	assert(m_avStream != nullptr);
	m_avStream->id = m_muxer->nb_streams - 1;
	m_avStream->time_base = m_encoder->time_base;

	// Some formats want stream headers to be separate.
	if (m_muxer->oformat->flags & AVFMT_GLOBALHEADER)
		m_encoder->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	assert(avcodec_parameters_from_context(m_avStream->codecpar, m_encoder) == 0);
	assert(avio_open(&m_muxer->pb, fileName.c_str(), AVIO_FLAG_WRITE) == 0);
	assert(avformat_write_header(m_muxer, nullptr) == 0);

	swsCtx = sws_getContext(
		width, height, SRC_PIX_FMT, // src
		width, height, DEST_PIX_FMT, // dst
		//SWS_FAST_BILINEAR, NULL, NULL, NULL);
		SWS_POINT, NULL, NULL, NULL);

	debug = Texture::LoadFromBuffer(ColorChannel::RGB, width, height);
}

Encoder::~Encoder()
{
	flush();
	delete debug;
	debug = nullptr;
	avio_closep(&m_muxer->pb);
	avcodec_close(m_encoder);
	avformat_free_context(m_muxer);
	sws_freeContext(swsCtx);
}

void Encoder::addFrame(unsigned char* pixels)
{
	AVFrame* frame = av_frame_alloc();
	frame->format = SRC_PIX_FMT;
	frame->width = m_encoder->width;
	frame->height = m_encoder->height;
	assert(av_frame_get_buffer(frame, 0) == 0);
	assert(av_frame_make_writable(frame) == 0);

	debug->UpdateBuffer(ColorChannel::RGB, frame->width, frame->height, pixels, false);

	for (int y = 0; y < frame->height; y++) {
		for (int x = 0; x < frame->width; x++) {
			int iy = frame->height - y - 1;
			frame->data[0][y * frame->linesize[0] + x + 0] = pixels[iy * 3 * frame->width + 3 * x + 2];
			frame->data[0][y * frame->linesize[0] + x + 1] = pixels[iy * 3 * frame->width + 3 * x + 1];
			frame->data[0][y * frame->linesize[0] + x + 2] = pixels[iy * 3 * frame->width + 3 * x + 0];
		}
	}

	AVFrame* yuvframe = av_frame_alloc();
	yuvframe->format = DEST_PIX_FMT;
	yuvframe->width = m_encoder->width;
	yuvframe->height = m_encoder->height;
	assert(av_frame_get_buffer(yuvframe, 0) == 0);
	assert(av_frame_make_writable(yuvframe) == 0);

	sws_scale(swsCtx, frame->data, frame->linesize, 0, m_encoder->height, yuvframe->data, yuvframe->linesize);

	yuvframe->pts = m_frameId++;
	encodeFrame(yuvframe);

	av_frame_free(&frame);
	av_frame_free(&yuvframe);
}

void Encoder::flush()
{
	encodeFrame(nullptr);
	av_write_trailer(m_muxer);
}

void Encoder::encodeFrame(AVFrame* frame)
{
	assert(avcodec_send_frame(m_encoder, frame) == 0);

	AVPacket* packet = av_packet_alloc();
	int ret = 0;
	while (ret >= 0) {
		ret = avcodec_receive_packet(m_encoder, packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return;  // nothing to write
		}
		assert(ret >= 0);

		av_packet_rescale_ts(packet, m_encoder->time_base, m_avStream->time_base);
		packet->stream_index = m_avStream->index;
		av_interleaved_write_frame(m_muxer, packet);
		av_packet_unref(packet);
	}
	av_packet_free(&packet);
}