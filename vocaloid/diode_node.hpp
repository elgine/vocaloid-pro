#pragma once
#include "audio_node.hpp"
#include "audio_param.hpp"
#include "wave_shaper_node.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		class DiodeNode : public WaveShaperNode {
		public:
			float vb_;
			float vl_;
			float h_;

			explicit DiodeNode(BaseAudioContext *ctx) :WaveShaperNode(ctx) {
				vb_ = 0.2;
				vl_ = 0.4;
				h_ = 1;
			}

			void UpdateCurve() {
				int i, _i, samples;
				float v,
					value,
					_ref,
					retVal;

				samples = 1024;

				DeleteArray(&curve_);
				AllocArray(samples, &curve_);

				for (i = _i = 0, _ref = samples; 0 <= _ref ? _i < _ref : _i > _ref; i = 0 <= _ref ? ++_i : --_i) {
					v = (i - samples / 2) / (samples / 2);
					v = abs(v);
					if (v <= vb_) {
						value = 0;
					}
					else if ((vb_ < v) && (v <= vl_)) {
						value = h_ * ((pow(v - vb_, 2)) / (2 * vl_ - 2 * vb_));
					}
					else {
						value = h_ * v - h_ * vl_ + (h_ * ((pow(vl_ - vb_, 2)) / (2 * vl_ - 2 * vb_)));
					}
					curve_[i] = value;
				}
			}
		};
	}
}