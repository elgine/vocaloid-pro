#pragma once
#include "stdafx.h"
#include "file.h"
#include "source_node.hpp"
#include "audio_context.hpp"
#ifdef WIN
#include "ffmpeg_io.hpp"
#else
#include "wav.hpp"
#endif
namespace vocaloid {
	namespace node {
		class FileReaderNode: public SourceNode {
		protected:
			string path_;
			io::AudioFileReader *reader_;
		private:
			float resample_ratio_;
			uint64_t require_buffer_size_;
			uint64_t require_float_size_;
			uint16_t bits_;
		public:
			explicit FileReaderNode(AudioContext *ctx) :SourceNode(ctx) {
				path_ = "undefined";
#ifdef WIN
				reader_ = new io::FFmpegFileReader();
#else
				reader_ = new io::WAVReader();
#endif

				reader_ = nullptr;
				resample_ratio_ = 1.0;
				bits_ = 16;
				require_buffer_size_ = require_float_size_ = 0;
			}

			void SetPath(const char* path) {
				path_ = path;
			}

			void Initialize(uint64_t frame_size) override {
				frame_size_ = frame_size;
				reader_->Open(path_.c_str());
				auto format = reader_->Format();
				channels_ = format.channels;
				bits_ = format.bits;
				if (format.sample_rate == 0)throw "The Sample rate of file is zero!";
				resample_ratio_ = float(context_->GetSampleRate()) / format.sample_rate;
				if (resample_ratio_ == 0)resample_ratio_ = 1.0f;
				require_float_size_ = uint64_t(float(frame_size_) / resample_ratio_);
				require_buffer_size_ = require_float_size_ * format.block_align;
			}

			int64_t Process(Frame *in) override {
				if (reader_->IsEnd() || !enable_) {
					in->Fill(0.0f);
					return frame_size_;
				}
				char *bytes = new char[require_buffer_size_];
				reader_->ReadData(bytes, require_buffer_size_);
				if (resample_ratio_ != 1.0) {
					input_buffer_->FromByteArray(bytes, require_buffer_size_, bits_, channels_);
					for (auto i = 0; i < channels_; i++) {
						Resample(input_buffer_->Channel(i)->Data(), require_float_size_, INTERPOLATOR_TYPE::LINEAR, resample_ratio_, in->Channel(i)->Data());
					}
				}
				else
					in->FromByteArray(bytes, require_buffer_size_, bits_, channels_);
				delete[] bytes;
				bytes = nullptr;
				return frame_size_;
			}
		};
	}
}