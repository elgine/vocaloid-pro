#pragma once
#include "./source_node.hpp"
namespace vocaloid {
	namespace node {

		class BufferNode : public SourceNode {	
		private:
			AudioFrame *buf_;
			int64_t played_began_;
			bool began_;
			int64_t start_point_;
			int64_t offset_point_;
			int64_t played_point_;
			int64_t duration_point_;
		public:

			bool loop_;

			explicit BufferNode(AudioContext *ctx):SourceNode(ctx) {
				buf_ = new AudioFrame();
				loop_ = false;
				start_point_ = 0;
				offset_point_ = 0;
				played_point_ = 0;
				duration_point_ = 0;
				played_began_ = 0;
				began_ = false;
			}

			void SetBuffer(AudioFrame *b) {
				buf_->Copy(b);
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				channels_ = buf_->Channels();
				SourceNode::Initialize(sample_rate, frame_size);
				played_point_ = offset_point_;
			}

			int64_t ProcessFrame() override {
				if (!loop_ && played_point_ - start_point_ >= duration_point_)return 0;
				if (!began_) {
					if (played_began_ < start_point_) {
						played_began_ += frame_size_;
						return frame_size_;
					}
					began_ = true;
					played_point_ = played_began_ - start_point_ + offset_point_;
				}
				int64_t size = 0;
				int64_t fill_size = 0;
				int64_t buffer_size = buf_->Size();
				while (size < frame_size_) {
					fill_size = min(frame_size_ - size, duration_point_ + offset_point_ - played_point_);
					for (auto i = 0; i < channels_; i++)
						result_buffer_->Data()[i]->Set(buf_->Channel(i)->Data(), fill_size, played_point_, size);
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
				auto sample_rate = context_->SampleRate();
				start_point_ = sample_rate * when * 0.001f;
				offset_point_ = sample_rate * offset * 0.001f;
				if (duration > 0)
					duration_point_ = sample_rate * duration * 0.001f;
				else
					duration_point_ = buf_->Size();
				began_ = false;
				played_began_ = 0;
			}
		};
	}
}