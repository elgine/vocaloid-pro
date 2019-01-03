#pragma once
#include "./source_node.hpp"
namespace vocaloid {
	namespace node {

		class BufferNode : public SourceNode {	
		public:

			explicit BufferNode(BaseAudioContext *ctx):SourceNode(ctx) {}

			void SetBuffer(AudioChannel *b) {
				summing_buffer_->Copy(b);
				summing_buffer_->silence_ = false;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				channels_ = summing_buffer_->Channels();
				SourceNode::Initialize(sample_rate, frame_size);
				return SUCCEED;
			}

			int64_t GetBuffer(int64_t offset, int64_t len) override {
				auto all_count = summing_buffer_->Size();
				auto bytes = 0, count = 0;
				auto index = play_pos_;
				while (bytes < len) {
					count = min(all_count, index + len) - index;
					for (auto i = 0; i < channels_; i++)
						result_buffer_->Data()[i]->Set(summing_buffer_->Channel(i)->Data(), count, index, offset + bytes);
					bytes += count;
					index += count;
					if (index >= all_count && !loop_) {
						return EOF;
					}
					index %= all_count;
				}
				result_buffer_->silence_ = false;
				return bytes;
			}

			int64_t Duration() override {
				return BytesToMsec(SourceSampleRate(), summing_buffer_->Size());
			}

			int32_t SourceSampleRate() override {
				return summing_buffer_->sample_rate_;
			}
		};
	}
}