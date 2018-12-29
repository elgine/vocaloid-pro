#pragma once
#include "wave_shaper_node.hpp"

namespace vocaloid {
	namespace node {
		
		class DistortionNode : public WaveShaperNode {

		private:

			void BuiltCurve() {
				auto gain = gain_ * 100;
				curve_length_ = sample_rate_;
				DeleteArray(&curve_);
				AllocArray(sample_rate_, &curve_);
				float deg = M_PI / 180;
				for (auto i = 0; i < sample_rate_; ++i) {
					auto x = i * 2.0f / sample_rate_ - 1.0f;
					curve_[i] = (3 + gain) * x * 20 * deg / (M_PI + gain * fabsf(x));
				}
			}

		public:

			float gain_;

			DistortionNode(AudioContext *ctx):WaveShaperNode(ctx) {
				gain_ = 0.5f;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) {
				WaveShaperNode::Initialize(sample_rate, frame_size);
				BuiltCurve();
				return SUCCEED;
			}
		};
	}
}