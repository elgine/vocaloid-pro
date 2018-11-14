#pragma once
#include "stdafx.h"
#include "file.h"
#include "source_node.hpp"
#include "audio_context.hpp"
#include "../utility/path.hpp"
#include "maths.hpp"
#ifdef _WIN32 || _WIN64
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
			char* temp_buffer_;
			float resample_ratio_;
			int64_t require_buffer_size_;
			int64_t require_float_size_;
			int16_t bits_;
		public:
			explicit FileReaderNode(AudioContext *ctx) :SourceNode(ctx) {
				path_ = "undefined";
#ifdef _WIN32 || _WIN64
				reader_ = new io::FFmpegFileReader();
#else
				reader_ = new io::WAVReader();
#endif
				resample_ratio_ = 1.0;
				bits_ = 16;
				require_buffer_size_ = require_float_size_ = 0;
			}

			void SetPath(const char* path) {
#ifndef _WIN32 || _WIN64
				if (GetExtension(path) != ".wav") {
					throw "Don't support audio file except 'wav'";
				}
#endif
				path_ = path;
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				SourceNode::Initialize(sample_rate, frame_size);
				frame_size_ = frame_size;
				reader_->Open(path_.c_str());
				auto format = reader_->Format();
				channels_ = format.channels;
				bits_ = format.bits;
				if (format.sample_rate == 0)throw "The Sample rate of file is zero!";
				resample_ratio_ = float(sample_rate) / format.sample_rate;
				require_float_size_ = int64_t(float(frame_size_) / resample_ratio_);
				require_buffer_size_ = require_float_size_ * format.block_align;
				AllocArray(require_buffer_size_, &temp_buffer_);
			}

			int64_t ProcessFrame() override {
				if (!enable_)return 0;
				int64_t size = 0;
				if (reader_->IsEnd() && !reader_->CapableToRead(require_buffer_size_)) {
					reader_->Flush(temp_buffer_, size);
					memset(temp_buffer_ + size, 0, require_buffer_size_ - size);
				}else {
					size = reader_->ReadData(temp_buffer_, require_buffer_size_);
				}
				if (size <= 0) {
					return 0;
				}
				if (resample_ratio_ != 1.0) {
					summing_buffer_->FromByteArray(temp_buffer_, require_buffer_size_, bits_, channels_);
					for (auto i = 0; i < channels_; i++) {
						Resample(summing_buffer_->Channel(i)->Data(), require_float_size_, INTERPOLATOR_TYPE::LINEAR, resample_ratio_, result_buffer_->Channel(i)->Data());
					}
				}
				else
					result_buffer_->FromByteArray(temp_buffer_, require_buffer_size_, bits_, channels_);
				result_buffer_->silence_ = false;
				return frame_size_;
			}
		};
	}
}