#pragma once
#include "audio_node.hpp"
#include "maths.hpp"
#include "timeline.hpp"
namespace vocaloid {
	namespace node {
		class InputNode : public AudioNode{
		protected:
			bool active_;
			// Loop bytes
			bool loop_;
			bool eof_;
		public:

			explicit InputNode(BaseAudioContext *ctx) :AudioNode(ctx, AudioNodeType::INPUT, false, true) {
				active_ = false;
				loop_ = false;
				eof_ = false;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				sample_rate_ = sample_rate;
				frame_size_ = frame_size;
				result_buffer_->Alloc(channels_, frame_size_);
				result_buffer_->SetSize(frame_size_);
				eof_ = false;
				return SUCCEED;
			}

			int64_t Process(bool flush = false) override {
				if (!active_)return 0;
				result_buffer_->Zero();
				if (eof_) {
					result_buffer_->silence_ = false;
					return frame_size_;
				}
				return ProcessFrame(flush);
			}

			void Loop(bool v) {
				loop_ = v;
				if (loop_) {
					active_ = true;
					eof_ = false;
				}
			}

			void Stop() override {
				active_ = false;
			}

			bool Eof() {
				return eof_;
			}

			virtual void Resume() {
				active_ = true;
			}

			virtual int Start() {
				active_ = true;
				return SUCCEED;
			}
		};
	}
}