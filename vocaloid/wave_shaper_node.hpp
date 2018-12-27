#pragma once
#include <math.h>
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class WaveShaperNode : public AudioNode {
		protected:
			float *curve_;
			int64_t curve_length_;
		public:
			explicit WaveShaperNode(AudioContext *ctx) :AudioNode(ctx) {
				curve_length_ = 0;
				curve_ = nullptr;
			}

			void SetCurve(float *c, int64_t len) {
				DeleteArray(&curve_);
				AllocArray(len, &curve_);
				memcpy(curve_, c, len * sizeof(float));
				curve_length_ = len;
			}

			int64_t ProcessFrame() override {
				for (auto j = 0; j < frame_size_; j++) {
					for (int16_t i = 0; i < channels_; i++) {
						auto input = summing_buffer_->Channel(i)->Data()[j];
						// Calculate an index based on input -1 -> +1 with 0 being at the center of the curve data.
						auto index = 0.5f * (curve_length_ - 1) * (input + 1);
						// Clip index to the input range of the curve.
						// This takes care of input outside of nominal range -1 -> +1
						if (index < 0)result_buffer_->Channel(i)->Data()[j] = curve_[0];
						else if (index >= curve_length_ - 1)
							result_buffer_->Channel(i)->Data()[j] = curve_[curve_length_ - 1];
						else {
							auto index1 = static_cast<int64_t>(index);
							auto index2 = index1 + 1;
							auto factor = index - index1;
							auto v1 = curve_[index1];
							auto v2 = curve_[index2];
							result_buffer_->Channel(i)->Data()[j] = (1.0f - factor) * v1 + factor * v2;
						}
					}
				}
				return frame_size_;
			}

			void Dispose() override {
				DeleteArray(&curve_);
				curve_length_ = 0;
			}

			int64_t CurveLength() {
				return curve_length_;
			}
		};
	}
}