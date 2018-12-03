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

			bool loop_;

			explicit BufferNode(AudioContext *ctx):SourceNode(ctx) {
				loop_ = false;
				start_ = 0;
				offset_ = 0;
				duration_ = 0;
				played_began_ = 0;
				start_point_ = offset_point_ = duration_point_ = 0;
				began_ = false;
			}

			void SetBuffer(AudioChannel *b) {
				summing_buffer_->Copy(b);
			}

			void SetChannels(int16_t c) final {
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				channels_ = summing_buffer_->Channels();
				SourceNode::Initialize(sample_rate, frame_size);
				start_point_ = sample_rate * start_ * 0.001f;
				offset_point_ = summing_buffer_->sample_rate_ * offset_ * 0.001f;
				duration_point_ = Clamp(int64_t(0), summing_buffer_->Size(), int64_t(summing_buffer_->sample_rate_ * duration_ * 0.001f));
				began_ = false;
				played_began_ = 0;
				played_point_ = offset_point_;
			}

			int64_t ProcessFrame() override {
				if (!loop_ && played_point_ - offset_point_ >= duration_point_) {
					if (!finished_)finished_ = true;
					return 0;
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

			void Start(int64_t when, int64_t offset = 0, int64_t duration = 0) {
				SourceNode::Start();
				start_ = when;
				offset_ = offset;
				duration_ = duration;
				auto sample_rate = context_->SampleRate();
			}


		};
	}
}