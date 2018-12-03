#pragma once
#include "audio_context.hpp"
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class SourceNode : public AudioNode {
		protected:
			bool played_;
			bool finished_;
		public:
			explicit SourceNode(AudioContext *ctx) :AudioNode(ctx, AudioProcessorType::INPUT, false, true) {
				played_ = false;
				finished_ = false;
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				finished_ = false;
			}

			int64_t Process() override {
				if (played_)
					return ProcessFrame();
				return 0;
			}

			bool Finished() override {
				return finished_;
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