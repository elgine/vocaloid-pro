#pragma once
#include "audio_node.hpp"
#include "audio_param.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		class SqueakNode : public AudioNode {

		public:

			float depth_;
			float master_gain_;
			int sample_period_;
			float noise_gain_;
			float squeak_mod_period_;
			float squeak_gain_;

			explicit SqueakNode(BaseAudioContext *ctx) :AudioNode(ctx) {
				depth_ = 1.0;
				master_gain_ = 0.5;
				sample_period_ = 7;
				noise_gain_ = 0.1;
				squeak_mod_period_ = 0.04;
				squeak_gain_ = 0.05;
			}

			int64_t ProcessFrame(bool flush = false) override {
				float sample = 0;
				float noise = 0;
				float squeak = 0;
				float squeakMod = rand() / 32767.0;
				for (auto i = 0; i < frame_size_; i++) {
					for (auto j = 0; j < channels_; j++) {
						if (i % sample_period_ == 0) { sample = ((floor(depth_ * summing_buffer_->Channel(j)->Data()[i]) / depth_) + noise + squeak) * master_gain_; }
						if (i % 7 == 0) { noise = rand() / 32767.0 * noise_gain_; }
						if (i % 700 == 0) { squeakMod += (rand() / 32767.0 * 2 - 1) * squeak_mod_period_; }
						squeak = sin(i * squeakMod * 1.5) * squeak_gain_;
						result_buffer_->Channel(j)->Data()[i] = sample;
					}
				}
				return frame_size_;
			}
		};
	}
}