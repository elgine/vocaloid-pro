#pragma once
#include "../vocaloid/wave_shaper_node.hpp"
#include "effect.hpp"
namespace effect {
		
	class Distortion : public Effect {
	private:

		WaveShaperNode *shaper_;
		float gain_;
		float* curve_;

		void UpdateCurve() {
			auto gain = gain_ * 100;
			auto curve_length = 44100;
			DeleteArray(&curve_);
			AllocArray(curve_length, &curve_);
			float deg = M_PI / 180;
			for (auto i = 0; i < curve_length; ++i) {
				auto x = i * 2.0f / curve_length - 1.0f;
				curve_[i] = (3 + gain) * x * 20 * deg / (M_PI + gain * fabsf(x));
			}

			shaper_->SetCurve(curve_, curve_length);
		}

	public:

		static float GAIN_DEFAULT;
		static float GAIN_MIN;
		static float GAIN_MAX;

		explicit Distortion(BaseAudioContext *ctx):Effect(ctx) {
			id_ = Effects::DISTORTION;
			shaper_ = new WaveShaperNode(ctx);
			gain_ = 0.5f;
			ctx->Connect(input_, shaper_);
			ctx->Connect(shaper_, wet_);
		}

		void SetOptions(float* options, int16_t option_count) override {
			if (option_count > 0) {
				SetGain(options[0]);
			}
			if (option_count > 1) {
				CrossFade(options[1]);
			}
		}

		void SetGain(float v) {
			v = Clamp(GAIN_MIN, GAIN_MAX, v);
			if (gain_ == v)return;
			UpdateCurve();
		}

		void Dispose() override {
			if (shaper_) {
				shaper_->Dispose();
				delete shaper_;
				shaper_ = nullptr;
			}
			Effect::Dispose();
		}
	};

	float Distortion::GAIN_DEFAULT = 0.5;
	float Distortion::GAIN_MIN = 0.0;
	float Distortion::GAIN_MAX = 1.0;
}