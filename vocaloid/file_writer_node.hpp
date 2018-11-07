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
		public:
			explicit FileWriterNode(AudioContext *ctx): DestinationNode(ctx){
#ifdef _WIN32 || _WIN64
				writer_ = new io::FFmpegFileWriter();
#else
				writer_ = new io::WAVWriter();
#endif
			}

			void Initialize(uint64_t frame_size) override {
				Node::Initialize(frame_size);
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

			int64_t PushToDestination() override {
				uint64_t size = summing_buffer_->Size();
				uint64_t byte_len = size * summing_buffer_->Channels() * BITS_PER_SEC / 8;
				auto bytes = new char[byte_len];
				summing_buffer_->ToByteArray(BITS_PER_SEC, bytes, byte_len);
				writer_->WriteData(bytes, byte_len);
				delete[] bytes;
				bytes = nullptr;
				return size;
			}
		};
	}
}