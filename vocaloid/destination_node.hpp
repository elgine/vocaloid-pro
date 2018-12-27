#pragma once
#include "audio_node.hpp"
namespace vocaloid {
	namespace node {
		class DestinationNode : public AudioNode {
		protected:
			int32_t sample_rate_ = 44100;
			OutputType output_type_;
		public:
			explicit DestinationNode(BaseAudioContext *ctx) :AudioNode(ctx, AudioNodeType::OUTPUT, true, false) {}

			void SetFormat(int32_t sample_rate, int16_t channels) {
				sample_rate_ = sample_rate;
				channels_ = channels;
			}

			virtual int64_t Processed() { return 0; }

			OutputType OutputType() {
				return output_type_;
			}
			
			int32_t SampleRate() {
				return sample_rate_;
			}
		};
	}
}