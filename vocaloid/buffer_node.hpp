#pragma once
#include "./source_node.hpp"
namespace vocaloid {
	namespace node {

		class BufferNode : public SourceNode {	
		public:

			explicit BufferNode(AudioContext *ctx):SourceNode(ctx) {}

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
				for (auto i = 0; i < channels_; i++)
					result_buffer_->Data()[i]->Set(summing_buffer_->Channel(i)->Data(), len, play_pos_, offset);
				return len;
			}

			int64_t Duration() {
				return summing_buffer_->Size() / sample_rate_ * 1000;
			}

			int32_t SourceSampleRate() override {
				return summing_buffer_->sample_rate_;
			}
		};
	}
}