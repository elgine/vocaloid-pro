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

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);;
			}

			int64_t Process() override {
				if (played_)
					return ProcessFrame();
				return 0;
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