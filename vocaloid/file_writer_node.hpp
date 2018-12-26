#pragma once
#include "file.h"
#ifdef _WIN32 || _WIN64
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
			Buffer<char> *buf_;
		public:
			explicit FileWriterNode(BaseAudioContext *ctx): DestinationNode(ctx){
#ifdef _WIN32 || _WIN64
				writer_ = new io::FFmpegFileWriter();
#else
				writer_ = new io::WAVWriter();
#endif
				buf_ = new Buffer<char>();
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				DestinationNode::Initialize(sample_rate, frame_size);
				writer_->Open(Path(), sample_rate_, BITS_PER_SEC, channels_);
			}

			void Close() override {
				writer_->Close();
			}

			void SetPath(const char* p) {
				path_ = p;
			}

			const char* Path() {
				return path_.c_str();
			}

			int64_t ProcessFrame() override {
				int64_t size = summing_buffer_->Size();
				int64_t byte_len = size * summing_buffer_->Channels() * BITS_PER_SEC / 8;
				buf_->Alloc(byte_len);
				buf_->SetSize(byte_len);
				summing_buffer_->ToByteArray(BITS_PER_SEC, buf_->Data(), byte_len);
				writer_->WriteData(buf_->Data(), byte_len);
				return size;
			}
		};
	}
}