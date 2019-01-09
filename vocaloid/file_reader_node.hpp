#pragma once
#include "stdafx.h"
#include "file.h"
#include "source_node.hpp"
#include "../utility/path.hpp"
#include "maths.hpp"
#include "status.h"
#ifdef WIN
#include "ffmpeg_io.hpp"
#else
#include "wav.hpp"
#endif

namespace vocaloid {
	namespace node {
		class FileReaderNode: public SourceNode{
		protected:
			string path_;
			io::AudioFileReader *reader_;
		private:
			char* temp_buffer_;
			float resample_ratio_;
			int16_t bits_;

			bool need_update_;
		public:

			explicit FileReaderNode(BaseAudioContext *ctx) :SourceNode(ctx) {
				path_ = "undefined";
#ifdef WIN
				reader_ = new io::FFmpegFileReader();
#else
				reader_ = new io::WAVReader();
#endif
				resample_ratio_ = 1.0;
				bits_ = 16;
				temp_buffer_ = nullptr;
			}

			int Open(const char* path) {
				if (path_ == path)return SUCCEED;
				if (IsPathDirectory(path))
					return PATH_NOT_FILE;
				if (!IsFileReadable(path))
					return FILE_NOT_READABLE;
#ifndef _WIN
				if (GetExtension(path) != ".wav") {
					return FILE_NOT_WAV;
				}
#endif
				path_ = path;
				reader_->Dispose();
				return reader_->Open(path_.c_str());
			}

			void Clear() override {
				SourceNode::Clear();
				reader_->Clear();
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				SourceNode::Initialize(sample_rate, frame_size);
				auto format = reader_->Format();
				channels_ = format.channels;
				bits_ = format.bits;
				if (format.sample_rate == 0) {
					return INVALIDATE_SOURCE;
				}
				DeleteArray(&temp_buffer_);
				AllocArray(int64_t(float(frame_size_) / resample_ratio_) * format.block_align, &temp_buffer_);
				summing_buffer_->sample_rate_ = format.sample_rate;
				return SUCCEED;
			}

			int64_t GetBuffer(int64_t offset, int64_t len) override {
				auto frame_size = BITS_PER_SEC / 8 * channels_;
				auto buf_size = len * frame_size;
				auto size = reader_->ReadData(temp_buffer_, buf_size);
				if (size <= 0) {
					if (reader_->End())return loop_?len:EOF;
					return 0;
				}
				result_buffer_->silence_ = false;
				if (resample_ratio_ != 1.0) {
					summing_buffer_->FromByteArray(temp_buffer_, buf_size, bits_, channels_);
					for (auto i = 0; i < channels_; i++) {
						Resample(summing_buffer_->Channel(i)->Data(), size, INTERPOLATOR_TYPE::LINEAR, resample_ratio_, result_buffer_->Channel(i)->Data() + offset);
					}
				}
				else
					result_buffer_->FromByteArray(temp_buffer_, buf_size, bits_, channels_, offset);
				return float(size) / frame_size;
			}

			int64_t SeekInternal(int64_t frame_offset) override {
				return reader_->Seek(frame_offset);
			}

			int32_t SourceSampleRate() override {
				return summing_buffer_->sample_rate_;
			}

			int64_t Duration() {
				return reader_->Duration();
			}

			void Stop() override {
				SourceNode::Stop();
				reader_->Stop();
			}

			void Dispose() override {
				AudioNode::Dispose();
				reader_->Dispose();
			}
		};
	}
}