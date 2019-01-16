#pragma once
#include "file.h"
#ifdef WIN
#include "ffmpeg_io.hpp"
#else
#include "wav.hpp"
#endif
#include "destination_node.hpp"
namespace vocaloid {
	namespace node {
		class FileWriterNode : public DestinationNode {
		private:
			io::AudioFileWriter *writer_;
			string path_;
			int64_t processed_;
			Buffer<char> *buf_;
		public:
			explicit FileWriterNode(BaseAudioContext *ctx): DestinationNode(ctx){
#ifdef WIN
				writer_ = new io::FFmpegFileWriter();
#else
				writer_ = new io::WAVWriter();
#endif
				buf_ = new Buffer<char>();
			}

			void Dispose() override {
				Close();
				if (writer_) {
					writer_->Dispose();
					delete writer_;
					writer_ = nullptr;
				}
				DestinationNode::Dispose();
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				DestinationNode::Initialize(sample_rate, frame_size);
				processed_ = 0;
				return writer_->Open(Path(), sample_rate_, BITS_PER_SEC, channels_);
			}

			void Close() override {
				if (writer_) {
					writer_->Dispose();
				}
			}

			void SetPath(const char* p) {
				path_ = p;
			}

			const char* Path() {
				return path_.c_str();
			}

			int64_t Processed() override {
				return processed_;
			}

			int64_t ProcessFrame(bool flush = false) override {
				int64_t size = summing_buffer_->Size();
				int64_t byte_len = size * summing_buffer_->Channels() * BITS_PER_SEC / 8;
				buf_->Alloc(byte_len);
				buf_->SetSize(byte_len);
				summing_buffer_->ToByteArray(BITS_PER_SEC, buf_->Data(), byte_len);
				writer_->WriteData(buf_->Data(), byte_len);
				processed_ += frame_size_;
				return size;
			}
		};
	}
}