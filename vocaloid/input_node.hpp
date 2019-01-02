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

			bool loop_;

			explicit InputNode(BaseAudioContext *ctx) :AudioNode(ctx, AudioNodeType::INPUT, false, true) {
				active_ = false;
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