#pragma once
#include "audio_node.hpp"
namespace vocaloid {
	namespace node {
		class InputNode : public AudioNode {
		protected:
			bool played_;
		public:

			bool loop_;

			explicit InputNode(BaseAudioContext *ctx) :AudioNode(ctx, AudioNodeType::INPUT, false, true) {
				played_ = false;
			}

			int64_t Process() override {
				if (!played_)return 0;
				result_buffer_->Zero();
				return ProcessFrame();
			}

			virtual void Seek(int64_t time) {}

			void Stop() override {
				played_ = false;
			}

			void Start() {
				played_ = true;
			}
		};
	}
}