#pragma once
#include "audio_context.hpp"
using namespace std;
namespace vocaloid {

	namespace node {
		class GainNode : public AudioNode {
		public:
			AudioParam *gain_;

			explicit GainNode(AudioContext *ctx, float v = 1.0f):AudioNode(ctx) {
				gain_ = new AudioParam();
				gain_->value_ = v;
			}

			void Initialize(uint32_t sample_rate, uint64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				gain_->Initialize(sample_rate_, frame_size);
			}

			int64_t ProcessFrame() override {
				auto gain_result = gain_->Result()->Channel(0)->Data();
				for (auto i = 0; i < frame_size_; i++) {
					auto value = gain_result[i];
					if (value < 0)value = 0;
					for (auto j = 0; j < channels_; j++) {
						result_buffer_->Channel(j)->Data()[i] = value * summing_buffer_->Channel(j)->Data()[i];
					}
				}
				return frame_size_;
			}
		};
	}
}