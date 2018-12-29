#pragma once
#include "stdafx.h"
#include "file.h"
#include "source_node.hpp"
#include "audio_context.hpp"
#include "../utility/path.hpp"
#include "maths.hpp"
#include "status.h"
#ifdef _WIN32 || _WIN64
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
			int64_t require_buffer_size_;
			int64_t require_float_size_;
			int16_t bits_;

			int64_t played_began_;
			bool began_;
			int64_t start_;
			int64_t offset_;
			int64_t duration_;

			int64_t start_point_;
			int64_t played_point_;
			int64_t offset_point_;
			int64_t duration_point_;

			bool need_update_;
		public:

			bool loop_;

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
				temp_buffer_ = nullptr;

				loop_ = false;
				start_point_ = 0;
				offset_point_ = 0;
				duration_point_ = 0;
			}

			int SetPath(const char* path) {
				if (path_ == path)return SUCCEED;
				if (IsPathDirectory(path))
					return PATH_NOT_FILE;
				if (!IsFileReadable(path))
					return FILE_NOT_READABLE;
#ifndef _WIN32 || _WIN64
				if (GetExtension(path) != ".wav") {
					return FILE_NOT_WAV;
				}
#endif
				path_ = path;
				need_update_ = true;
			}

			void Reset() override {
				played_point_ = 0;
				played_began_ = 0;
				began_ = false;
				reader_->Seek(0);
			}

			void Seek(int64_t time) override {
				played_point_ = time * 0.001 * sample_rate_;
				reader_->Seek(time);
			}

			void Clear() override {
				reader_->Clear();
			}

			// TODO: Loop and play in segments...
			void Start(int64_t when, int64_t offset = 0, int64_t duration = 0) {
				SourceNode::Start();
				start_ = when;
				offset_ = offset;
				duration_ = duration;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				SourceNode::Initialize(sample_rate, frame_size);
				frame_size_ = frame_size;
				auto ret = SUCCEED;
				if (need_update_) {
					reader_->Dispose();
					ret = reader_->Open(path_.c_str());
					if (ret < 0) {
						return CANT_DECODE;
					}
					need_update_ = false;
				}
				auto format = reader_->Format();
				channels_ = format.channels;
				bits_ = format.bits;
				if (format.sample_rate == 0) {
					return INVALIDATE_SOURCE;
				}
				resample_ratio_ = float(sample_rate) / format.sample_rate;
				require_float_size_ = int64_t(float(frame_size_) / resample_ratio_);
				require_buffer_size_ = require_float_size_ * format.block_align;
				DeleteArray(&temp_buffer_);
				AllocArray(require_buffer_size_, &temp_buffer_);

				began_ = false;
				played_began_ = 0;
				start_point_ = sample_rate * start_ * 0.001;
				offset_point_ = format.sample_rate * offset_ * 0.001;
				duration_point_ = format.sample_rate * duration_ * 0.001;
				if (duration_point_ <= 0)
					duration_point_ = reader_->FileLength() / (BITS_PER_SEC / 8) / channels_;
				played_point_ = offset_point_;
				summing_buffer_->sample_rate_ = format.sample_rate;
				return SUCCEED;
			}

			int64_t ProcessFrame() override {
				int64_t size = 0;
				if ((played_point_ - offset_point_ >= duration_point_)) {
					if (!loop_) {
						return EOF;
					}
					else {
						played_point_ = offset_point_;
						reader_->Seek(int64_t(float(played_point_) / sample_rate_ * 1000));
					}
				}
				if (!began_) {
					if (played_began_ < start_point_) {
						played_began_ += frame_size_;
						return 0;
					}
					auto diff = int64_t(float(played_began_ - start_point_) / sample_rate_ * 1000);
					auto offset_time = offset_ + diff;
					reader_->Seek(offset_time);
					began_ = true;
				}

				if (reader_->IsEnd() && !reader_->CapableToRead(require_buffer_size_)) {
					size = require_buffer_size_;
					reader_->Flush(temp_buffer_, size);
					if (size <= 0) {
						if (!loop_) {
							return EOF;
						}
						else {
							reader_->Seek(int64_t(float(offset_point_) / sample_rate_ * 1000));
						}
					}else {
						memset(temp_buffer_ + size, 0, require_buffer_size_ - size);
					}
				}
				
				size = reader_->ReadData(temp_buffer_, require_buffer_size_);
				if (size <= 0) {
					return 0;
				}
				result_buffer_->silence_ = false;
				if (resample_ratio_ != 1.0) {
					summing_buffer_->FromByteArray(temp_buffer_, require_buffer_size_, bits_, channels_);
					for (auto i = 0; i < channels_; i++) {
						Resample(summing_buffer_->Channel(i)->Data(), require_float_size_, INTERPOLATOR_TYPE::LINEAR, resample_ratio_, result_buffer_->Channel(i)->Data());
					}
				}
				else
					result_buffer_->FromByteArray(temp_buffer_, require_buffer_size_, bits_, channels_);
				played_point_ += frame_size_;
				return frame_size_;
			}

			void Stop() override {
				reader_->Stop();
			}

			void Dispose() override {
				reader_->Dispose();
			}
		};
	}
}