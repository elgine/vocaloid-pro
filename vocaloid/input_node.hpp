#pragma once
#include "audio_node.hpp"
#include "maths.hpp"
#include "timeline.hpp"
namespace vocaloid {
	namespace node {
		class InputNode : public AudioNode{
		protected:
			bool active_;
		public:

			// Loop bytes
			bool loop_;

			// Watched for detecting source's eof
			bool watched_;

			explicit InputNode(BaseAudioContext *ctx) :AudioNode(ctx, AudioNodeType::INPUT, false, true) {
				active_ = false;
				loop_ = false;
				watched_ = false;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				sample_rate_ = sample_rate;
				frame_size_ = frame_size;
				result_buffer_->Alloc(channels_, frame_size_);
				result_buffer_->SetSize(frame_size_);
				return SUCCEED;
			}

			int64_t Process() override {
				if (!active_)return 0;
				result_buffer_->Zero();
				return ProcessFrame();
			}

			void Stop() override {
				active_ = false;
			}

			virtual int Start() {
				active_ = true;
				return SUCCEED;
			}
		};
	}
}