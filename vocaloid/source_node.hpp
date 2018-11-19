#pragma once
#include "audio_context.hpp"
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class SourceNode : public AudioNode {
		protected:
			bool played_;
		public:
			explicit SourceNode(AudioContext *ctx) :AudioNode(ctx, AudioProcessorType::INPUT, false, true) {
				played_ = false;
			}

			int64_t Process() override {
				if (played_)
					return AudioNode::Process();
				return 0;
			}
		
			void Stop() override {
				played_ = false;
			}

			void Start() {
				played_ = true;
			}
		};
	}
}