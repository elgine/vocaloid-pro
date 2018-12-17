#pragma once
#ifdef _WIN32 || _WIN64
#include "file.h"
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include <condition_variable>
#ifdef _WIN64
extern "C"
{
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avutil.h"
#include "libavutil/fifo.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")
}
#else
extern "C"
{
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avutil.h"
#include "libavutil/fifo.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")
};
#endif
namespace vocaloid {
	namespace io {
		class FFmpegFileReader : public AudioFileReader {
		private:
			AVFormatContext *ctx_;
			AVCodecContext *codec_ctx_;
			AVPacket *packet_;
			AVFrame *frame_;
			AVFrame *decode_frame_;
			SwrContext *swr_ctx_;
			int16_t a_stream_index_;

			thread *decode_thread_;
			mutex decode_mutex_;
			condition_variable can_decode_;
			atomic<bool> is_end_;
			atomic<bool> decoding_;
			atomic<bool> has_begun_;

			int64_t frame_count_;
			int64_t output_frame_size_;
			char *buffer_;
			int64_t buffer_size_;
			// The max buffer size
			int64_t max_buffer_size_;

			int Decode(AVPacket *packet, unique_lock<mutex> &lck) {
				auto got_frame = 0;
				auto ret = avcodec_decode_audio4(codec_ctx_, frame_, &got_frame, packet);
				if (got_frame > 0 && frame_->nb_samples == codec_ctx_->frame_size) {
					frame_count_++;
					Convert((const uint8_t**)frame_->data, frame_->nb_samples, lck);
				}
				FlushConvertor(lck);
				return ret;
			}

			int Convert(const uint8_t** frame_data, int64_t nb_samples, unique_lock<mutex> &lck) {
				auto samples = swr_convert(swr_ctx_, decode_frame_->data, decode_frame_->nb_samples,
					frame_data, nb_samples);
				if (samples > 0) {
					while (!EnableToDecode())
						can_decode_.wait(lck);
					memcpy(buffer_ + buffer_size_, decode_frame_->data[0], samples * 4);
					buffer_size_ += output_frame_size_;
				}
				return samples;
			}

			// Use swresample to resample data, if input buffer size
			// larger than output size, it would cache in it's inner
			// buffer queue. So, use swr_get_out_samples to get the 
			// queue size, if it's capable for output, pop buffer and loop.
			void FlushConvertor(unique_lock<mutex> &lck) {
				int fifo_size = swr_get_out_samples(swr_ctx_, 0);
				while (fifo_size >= decode_frame_->nb_samples) {
					auto samples = Convert(nullptr, 0, lck);
					fifo_size -= samples;
				}
			}

			// Some audio decoders decode only part of the packet, and have to be
			// called again with the remainder of the packet data.
			// Also, some decoders might over-read the packet.
			void DecodePacket(AVPacket *packet, unique_lock<mutex> &lck) {
				auto decoded = 0;
				while (packet->size > 0) {
					decoded = Decode(packet, lck);
					if (decoded < 0)break;
					decoded = FFMIN(decoded, packet->size);
					if (packet->size - decoded >= 0) {
						packet->data += decoded;
						packet->size -= decoded;
					}
					else {
						break;
					}
				}
			}

			void Loop() {
				while (true) {
					{
						unique_lock<mutex> lck(decode_mutex_);
						if (!decoding_)break;
						while (!EnableToDecode() && decoding_) {
#ifdef _DEBUG
							//cout << "waiting for pick buffer" << endl;
#endif
							can_decode_.wait(lck);
						}
						auto ret = av_read_frame(ctx_, packet_);
						if (ret == AVERROR_EOF) {
							packet_->data = nullptr;
							packet_->size = 0;
							Decode(packet_, lck);
							is_end_ = true;
							break;
						}
						else if(ret >= 0){
							if (packet_->stream_index == a_stream_index_) {
								DecodePacket(packet_, lck);
							}
						}
						av_packet_unref(packet_);
					}
				}
#ifdef _DEBUG
				//cout << "exist decoding thread" << endl;
#endif
				return;
			}

			bool EnableToDecode() {
				return max_buffer_size_ > buffer_size_ + output_frame_size_;
			}

		public:

			FFmpegFileReader(int64_t max_size = MAX_FFT_SIZE * 16) {
				max_buffer_size_ = max_size;
				buffer_ = new char[max_buffer_size_];
			}

			void SetMaxBufferSize(int64_t max_size) {
				max_buffer_size_ = max_size;
				DeleteArray(&buffer_);
				buffer_ = new char[max_buffer_size_];
			}

			int64_t FileLength() override {
				unique_lock<mutex> lck(decode_mutex_);
				auto audio_stream = ctx_->streams[a_stream_index_];
				auto duration = audio_stream->duration * av_q2d(audio_stream->time_base);
				return duration * codec_ctx_->sample_rate * codec_ctx_->channels * BITS_PER_SEC/8;
			}

			void Flush(char* data, int64_t& length) override {
				unique_lock<mutex> lck(decode_mutex_);
				memcpy(data, buffer_, buffer_size_);
				length = buffer_size_;
			}

			int64_t ReadData(char* data, int64_t length) override {
				if (!has_begun_) {
					has_begun_ = true;
					decoding_ = true;
					decode_thread_ = new thread(&FFmpegFileReader::Loop, this);
					return 0;
				}
				else {
					{
						unique_lock<mutex> lck(decode_mutex_);
						if (buffer_size_ < length) {
#ifdef _DEBUG
							//cout << "buffer size is too small: " << buffer_size_ << endl;
#endif
							return 0;
						}

						try {
							memcpy(data, buffer_, length);
							buffer_size_ -= length;
							if (buffer_size_ < 0)
								buffer_size_ = 0;
							if (buffer_size_ > 0)
								memmove(buffer_, buffer_ + length, buffer_size_);
							// memcpy(buffer_, buffer_ + length, buffer_size_);
						}
						catch (exception e) {
							cout << e.what() << endl;
						}

						if (EnableToDecode())
							can_decode_.notify_all();
#ifdef _DEBUG
						//cout << "Pick buffer" << endl;
#endif
					}
					return length;
				}
			}

			int16_t Open(const char *input_path) override {
				int16_t ret = 0;
				char* err_msg = new char[512];
				av_register_all();
				ctx_ = avformat_alloc_context();
				ret = avformat_open_input(&ctx_, input_path, NULL, NULL);
				if (ret < 0) {
					av_strerror(ret, err_msg, 512);
					// TODO: Log error
					return ret;
				}
				ret = avformat_find_stream_info(ctx_, NULL);
				if (ret < 0) {
					// TODO: Log error
					av_strerror(ret, err_msg, 512);
					return ret;
				}

				a_stream_index_ = -1;
				for (auto i = 0; i < ctx_->nb_streams; i++) {
					if (ctx_->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
						a_stream_index_ = i;
						break;
					}
				}

				if (a_stream_index_ == -1) {
					// TODO: Log error
					// Cant' find audio stream
					return -1;
				}

				codec_ctx_ = ctx_->streams[a_stream_index_]->codec;
				AVCodec *codec = avcodec_find_decoder(codec_ctx_->codec_id);
				if (codec == nullptr) {
					// TODO: Log error
					// Can't find audio decoder codec
					return -1;
				}

				ret = avcodec_open2(codec_ctx_, codec, nullptr);
				if (ret < 0) {
					// TODO: Log error
					// Can't open decoder
					av_strerror(ret, err_msg, 512);
					return -1;
				}

				if (codec_ctx_->frame_size <= 0)
					codec_ctx_->frame_size = 1024;

				swr_ctx_ = swr_alloc();
				enum AVSampleFormat in_sample_fmt = codec_ctx_->sample_fmt;
				enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
				auto in_sample_rate = codec_ctx_->sample_rate;
				auto out_sample_rate = 44100;
				int64_t in_ch_layout = codec_ctx_->channel_layout > 0 ? codec_ctx_->channel_layout : av_get_default_channel_layout(codec_ctx_->channels);
				int64_t out_ch_layout = in_ch_layout;

				swr_alloc_set_opts(swr_ctx_, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, nullptr);
				swr_init(swr_ctx_);

				packet_ = (AVPacket*)av_malloc(sizeof(AVPacket));
				packet_->data = nullptr;
				packet_->size = 0;
				frame_ = av_frame_alloc();

				// Initialize decode frame
				decode_frame_ = av_frame_alloc();
				decode_frame_->nb_samples = 1024;
				decode_frame_->format = out_sample_fmt;
				decode_frame_->channels = av_get_channel_layout_nb_channels(out_ch_layout);
				decode_frame_->channel_layout = out_ch_layout;
				decode_frame_->sample_rate = out_sample_rate;

				output_frame_size_ = av_samples_get_buffer_size(nullptr, decode_frame_->channels, decode_frame_->nb_samples,
					(AVSampleFormat)decode_frame_->format, 1);

				uint8_t* frame_buf = (uint8_t *)av_malloc(output_frame_size_);
				avcodec_fill_audio_frame(decode_frame_, decode_frame_->channels, (AVSampleFormat)decode_frame_->format, (const uint8_t*)frame_buf, output_frame_size_, 1);

				memset(buffer_, 0, max_buffer_size_);
				buffer_size_ = 0;
				frame_count_ = 0;
				has_begun_ = false;
				decoding_ = false;
				is_end_ = false;
				return ret;
			}

			void Stop() override {
				{
					unique_lock<mutex> lck(decode_mutex_);
					decoding_ = false;
					can_decode_.notify_all();
				}
				if (decode_thread_->joinable())
					decode_thread_->join();
			}

			void Close() override {
				av_packet_free(&packet_);
				av_frame_free(&frame_);
				av_frame_free(&decode_frame_);
				swr_free(&swr_ctx_);
				avcodec_close(codec_ctx_);
				avformat_close_input(&ctx_);
				avformat_free_context(ctx_);
			}

			bool IsEnd() override {
				return is_end_;
			}

			bool CapableToRead(int64_t len) {
				unique_lock<mutex> lck(decode_mutex_);
				return buffer_size_ >= len;
			}

			int64_t Seek(int64_t time) override {
				unique_lock<mutex> lck(decode_mutex_);
				//auto durationPerFrame = float(codec_ctx_->frame_size * AV_TIME_BASE) / codec_ctx_->sample_rate;
				//av_seek_frame(ctx_, a_stream_index_, floor(time / durationPerFrame), AVSEEK_FLAG_ANY);
				av_seek_frame(ctx_, a_stream_index_, int64_t(time * 0.001f / av_q2d(ctx_->streams[a_stream_index_]->time_base)), AVSEEK_FLAG_BACKWARD);
				avcodec_flush_buffers(ctx_->streams[a_stream_index_]->codec);
				if (is_end_) {
					is_end_ = false;
					has_begun_ = false;
				}
				return time;
			}

			AudioFormat Format() override {
				unique_lock<mutex> lck(decode_mutex_);
				int16_t bits = 16;
				int32_t sample_rate = decode_frame_->sample_rate;
				int16_t channels = decode_frame_->channels;
				int16_t block_align = bits / 8 * channels;
				return{
					sample_rate,
					bits,
					channels,
					block_align
				};
			}
		};

		class FFmpegFileWriter : public AudioFileWriter {
		private:
			AVFormatContext *ctx_;
			AVCodecContext *codec_ctx_;
			SwrContext *swr_ctx_;
			AVPacket *packet_;
			AVFrame *frame_;
			AVFrame *enc_frame_;
			int64_t frame_index_;
			char *buffer_;
			int64_t buffer_size_;
			int64_t max_buffer_size_;
			uint8_t *frame_buf_;
			int64_t frame_buffer_size_;

			void PushPlanarData(const char* planar_bytes, int64_t byte_length) {
				if (max_buffer_size_ < buffer_size_ + byte_length) {
					auto new_buffer = new char[buffer_size_ + byte_length];
					memcpy(new_buffer, buffer_, buffer_size_);
					memcpy(new_buffer + buffer_size_, planar_bytes, byte_length);
					DeleteArray(&buffer_);
					buffer_ = new_buffer;
					max_buffer_size_ = buffer_size_ + byte_length;
				}
				else {
					memcpy(buffer_ + buffer_size_, planar_bytes, byte_length);
				}
				buffer_size_ += byte_length;
			}

			int Encode(const uint8_t** frame_data, int64_t nb_samples) {
				auto samples = swr_convert(swr_ctx_, enc_frame_->data, frame_->nb_samples,
					frame_data, nb_samples);
				// Tell encode to use input frames' nb_sample to encode
				// output frame
				enc_frame_->nb_samples = frame_->nb_samples;
				codec_ctx_->frame_size = frame_->nb_samples;

				if (samples > 0) {
					auto ret = EncodeFrame();
					if (ret < 0)return ret;
				}
				return samples;
			}

			void FlushConvertor() {
				int fifo_size = swr_get_out_samples(swr_ctx_, 0);
				while (fifo_size >= enc_frame_->nb_samples) {
					auto samples = Encode(nullptr, 0);
					if (samples < 0)break;
					fifo_size -= samples;
				}
			}

			int EncodeFrame() {
				auto got_packet = 0;
				auto ret = avcodec_encode_audio2(codec_ctx_, packet_, enc_frame_, &got_packet);
				if (ret < 0) {
					printf("Failed to encode audio!\n");
					return ret;
				}
				if (got_packet == 1) {
					packet_->stream_index = 0;
					long pts = frame_index_ * codec_ctx_->frame_size;
					packet_->pts = pts;
					packet_->dts = pts;
					packet_->duration = codec_ctx_->frame_size;
					ret = av_write_frame(ctx_, packet_);
					av_packet_unref(packet_);
					frame_index_++;
				}
				return ret;
			}

		public:

			FFmpegFileWriter(int64_t init_size = 16384) {
				buffer_ = new char[init_size];
				max_buffer_size_ = init_size;
				buffer_size_ = 0;
			}

			int16_t Open(const char* output_path, int32_t sample_rate, int16_t bits, int16_t channels) override {
				av_register_all();
				auto ret = avformat_alloc_output_context2(&ctx_, nullptr, nullptr, output_path);
				if (ret < 0) {
					// TODO: Log error
					return ret;
				}
				auto format_ctx = av_guess_format(nullptr, output_path, nullptr);
				ctx_->oformat = format_ctx;

				AVCodecID codec_id = ctx_->oformat->audio_codec;
				AVCodec *codec = avcodec_find_encoder(codec_id);
				if (!codec) {
					printf("Codec not found\n");
					return -1;
				}

				// Alloc context
				codec_ctx_ = avcodec_alloc_context3(codec);
				if (!codec_ctx_) {
					printf("Could not allocate audio codec context\n");
					return -1;
				}

				// Add stream
				auto stream = avformat_new_stream(ctx_, codec);
				stream->id = stream->index = 0;
				

				codec_ctx_->codec_id = codec_id;
				codec_ctx_->codec_type = AVMEDIA_TYPE_AUDIO;
				
				codec_ctx_->sample_fmt = codec->sample_fmts[0];
				codec_ctx_->sample_rate = sample_rate;
				codec_ctx_->channel_layout = av_get_default_channel_layout(channels);
				codec_ctx_->channels = channels;
				codec_ctx_->bit_rate = sample_rate * channels * bits / 8;
				codec_ctx_->time_base.num = 1;
				codec_ctx_->time_base.den = codec_ctx_->sample_rate;

				stream->time_base.num = 1;
				stream->time_base.den = codec_ctx_->sample_rate;
				stream->codec = codec_ctx_;

				if (ctx_->oformat->flags & AVFMT_GLOBALHEADER)
					codec_ctx_->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

				if ((ret = avcodec_open2(codec_ctx_, codec, nullptr)) < 0) {
					printf("Could not open audio codec\n");
					return -1;
				}

				if ((ret = avcodec_parameters_from_context(stream->codecpar, codec_ctx_)) < 0) {
					printf("Could not copy the stream parameters\n");
					return -1;
				}

				av_dump_format(ctx_, 0, output_path, 1);

				/* open the output file, if needed */
				if (!(ctx_->flags & AVFMT_NOFILE)) {
					ret = avio_open(&ctx_->pb, output_path, AVIO_FLAG_WRITE);
					if (ret < 0) {
						printf("Could not open '%s'\n", output_path);
						return ret;
					}
				}

				/* Write the stream header, if any. */
				ret = avformat_write_header(ctx_, NULL);
				if (ret < 0) {
					printf("Error occurred when opening output file\n");
					return -1;
				}

				frame_index_ = 0;
				buffer_size_ = 0;

				// Don't let frame size be zero!
				if(codec_ctx_->frame_size <= 0)
					codec_ctx_->frame_size = 1024;

				packet_ = av_packet_alloc();

				frame_ = av_frame_alloc();
				frame_->nb_samples = 1024;
				frame_->format = AVSampleFormat::AV_SAMPLE_FMT_S16;
				frame_->channels = channels;
				frame_->channel_layout = av_get_default_channel_layout(channels);
				frame_->sample_rate = sample_rate;
				frame_buffer_size_ = av_samples_get_buffer_size(nullptr, frame_->channels, frame_->nb_samples,
					(AVSampleFormat)frame_->format, 1);
				frame_buf_ = (uint8_t *)av_malloc(frame_buffer_size_);
				avcodec_fill_audio_frame(frame_, frame_->channels, (AVSampleFormat)frame_->format, (const uint8_t*)frame_buf_, frame_buffer_size_, 1);

				enc_frame_ = av_frame_alloc();
				enc_frame_->nb_samples = codec_ctx_->frame_size;
				enc_frame_->format = codec_ctx_->sample_fmt;
				enc_frame_->channels = channels;
				enc_frame_->channel_layout = av_get_default_channel_layout(channels);
				enc_frame_->sample_rate = sample_rate;
				av_frame_get_buffer(enc_frame_, 1);

				enum AVSampleFormat in_sample_fmt = (AVSampleFormat)frame_->format;
				enum AVSampleFormat out_sample_fmt = (AVSampleFormat)enc_frame_->format;
				auto in_sample_rate = frame_->sample_rate;
				auto out_sample_rate = enc_frame_->sample_rate;
				int64_t in_ch_layout = frame_->channel_layout;
				int64_t out_ch_layout = enc_frame_->channel_layout;

				swr_ctx_ = swr_alloc_set_opts(nullptr, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate, 0, nullptr);
				ret = swr_init(swr_ctx_);
				if (ret < 0) {
					printf("Init exception\n");
					return ret;
				}
				return 0;
			}

			int64_t WriteData(const char* bytes, int64_t byte_length) override {
				PushPlanarData(bytes, byte_length);
				int64_t pos = 0;
				int64_t len = 0;
				auto ret = 0;
				while (buffer_size_ >= frame_buffer_size_) {
					memcpy(frame_buf_, buffer_ + pos, frame_buffer_size_);
					avcodec_fill_audio_frame(enc_frame_, enc_frame_->channels, (AVSampleFormat)enc_frame_->format, (const uint8_t*)frame_buf_, frame_buffer_size_, 1);
					ret = Encode((const uint8_t**)frame_->data, frame_->nb_samples);
					FlushConvertor();
					if (ret < 0) {
						return ret;
					}
					pos += frame_buffer_size_;
					buffer_size_ -= frame_buffer_size_;
				}
				memcpy(buffer_, buffer_ + pos, buffer_size_);
				return byte_length;
			}

			void Flush() {
				int ret = 0, got_packet = 0;
				while (true) {
					ret = avcodec_encode_audio2(codec_ctx_, packet_, nullptr, &got_packet);
					if (ret < 0)
						break;
					if (!got_packet) {
						ret = 0;
						break;
					}
					packet_->stream_index = 0;
					long pts = frame_index_ * codec_ctx_->frame_size;
					packet_->pts = pts;
					packet_->dts = pts;
					packet_->duration = codec_ctx_->frame_size;
					ret = av_interleaved_write_frame(ctx_, packet_);
					av_packet_unref(packet_);
					frame_index_++;
					if (ret < 0)
						break;
				}
			}

			void Close() {
				Flush();
				av_write_trailer(ctx_);
				avcodec_free_context(&codec_ctx_);
				av_packet_free(&packet_);
				av_frame_free(&enc_frame_);
				avio_close(ctx_->pb);
				//avformat_free_context(ctx_);
			}
		};
	}
}
#endif