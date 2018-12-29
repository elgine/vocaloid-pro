#pragma once
#include "./source_node.hpp"
namespace vocaloid {
	namespace node {

		class BufferNode : public SourceNode {	
		private:
			int64_t played_began_;
			bool began_;
			int64_t start_;
			int64_t offset_;
			int64_t duration_;

			int64_t start_point_;
			int64_t played_point_;
			int64_t offset_point_;
			int64_t duration_point_;
		public:

			explicit BufferNode(AudioContext *ctx):SourceNode(ctx) {}

			void SetBuffer(AudioChannel *b) {
				summing_buffer_->Copy(b);
				summing_buffer_->silence_ = false;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				channels_ = summing_buffer_->Channels();
				SourceNode::Initialize(sample_rate, frame_size);
				start_point_ = sample_rate * start_ * 0.001f;
				offset_point_ = summing_buffer_->sample_rate_ * offset_ * 0.001f;
				duration_point_ = Clamp(int64_t(0), summing_buffer_->Size(), int64_t(summing_buffer_->sample_rate_ * duration_ * 0.001f));
				if (duration_point_ == 0) {
					duration_point_ = summing_buffer_->Size();
				}
				began_ = false;
				played_began_ = 0;
				played_point_ = offset_point_;
				return SUCCEED;
			}

			int64_t ProcessFrame() override {
				if (played_point_ - offset_point_ >= duration_point_) {
					if (!loop_) {
						result_buffer_->silence_ = true;
						return EOF;
					}
					else {
						played_point_ = offset_point_ + (played_point_ - offset_point_) % duration_point_;
					}
				}
				if (!began_) {
					if (played_began_ < start_point_) {
						played_began_ += frame_size_;
						return frame_size_;
					}
					began_ = true;
					played_point_ = int64_t(float(played_began_ - start_point_) / sample_rate_ * summing_buffer_->sample_rate_) + offset_point_;
				}
				int64_t size = 0;
				int64_t fill_size = 0;
				int64_t buffer_size = summing_buffer_->Size();
				result_buffer_->silence_ = false;
				while (size < frame_size_) {
					fill_size = min(frame_size_ - size, duration_point_ + offset_point_ - played_point_);
					for (auto i = 0; i < channels_; i++)
						result_buffer_->Data()[i]->Set(summing_buffer_->Channel(i)->Data(), fill_size, played_point_, size);
					size += fill_size;
					played_point_ += fill_size;
					if (played_point_ - offset_point_ >= duration_point_) {
						if (!loop_)break;
						played_point_ = offset_point_ + (played_point_ - offset_point_) % duration_point_;
					}
				}
				return size;
			}
		};
	}
}