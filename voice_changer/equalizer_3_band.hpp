#pragma once
#include "../vocaloid/biquad_node.hpp"
#include "effect.hpp"
namespace effect {
	using namespace vocaloid;
	using namespace vocaloid::node;
	using namespace vocaloid::dsp;

	class Equalizer3Band : public Effect {
	public:

		static float LOWSHELF_GAIN_DEFAULT;
		static float LOWSHELF_GAIN_MIN;
		static float LOWSHELF_GAIN_MAX;

		static float PEAKING_GAIN_DEFAULT;
		static float PEAKING_GAIN_MIN;
		static float PEAKING_GAIN_MAX;

		static float HIGHSHELF_GAIN_DEFAULT;
		static float HIGHSHELF_GAIN_MIN;
		static float HIGHSHELF_GAIN_MAX;

		struct Equalizer3BandOptions {
			float peaking_gain;
			float lowshelf_gain;
			float highshelf_gain;

			Equalizer3BandOptions(float lo_gain, float mi_gain, float hi_gain) {
				peaking_gain = mi_gain;
				lowshelf_gain = lo_gain;
				highshelf_gain = hi_gain;
			}
		};

		BiquadNode *hi_;
		BiquadNode *lo_;
		BiquadNode *mi_;

		explicit Equalizer3Band(BaseAudioContext *ctx):Effect(ctx){
			id_ = Effects::EQUALIZER_3_BAND;
			hi_ = new BiquadNode(ctx);
			hi_->type_ = BIQUAD_TYPE::HIGH_SHELF;
			hi_->frequency_->value_ = 2500;
			hi_->gain_->value_ = -10;
			lo_ = new BiquadNode(ctx);
			lo_->type_ = BIQUAD_TYPE::LOW_SHELF;
			lo_->frequency_->value_ = 100;
			lo_->gain_->value_ = 1;
			mi_ = new BiquadNode(ctx);
			mi_->type_ = BIQUAD_TYPE::PEAKING;
			mi_->frequency_->value_ = 900;
			mi_->gain_->value_ = -7;
			ctx->Connect(input_, hi_);
			ctx->Connect(hi_, mi_);
			ctx->Connect(mi_, lo_);
			ctx->Connect(lo_, wet_);

			SetLowShelfGain(LOWSHELF_GAIN_DEFAULT);
			SetPeakingGain(PEAKING_GAIN_DEFAULT);
			SetHighShelfGain(HIGHSHELF_GAIN_DEFAULT);
		}

		void Dispose() override {
			if (hi_) {
				hi_->Dispose();
				delete hi_;
				hi_ = nullptr;
			}
			if (lo_) {
				lo_->Dispose();
				delete lo_;
				lo_ = nullptr;
			}
			if (mi_) {
				mi_->Dispose();
				delete mi_;
				mi_ = nullptr;
			}
			Effect::Dispose();
		}

		void SetLowShelfGain(float v) {
			lo_->gain_->value_ = Clamp(LOWSHELF_GAIN_MIN, LOWSHELF_GAIN_MAX, v);
		}

		void SetHighShelfGain(float v) {
			hi_->gain_->value_ = Clamp(HIGHSHELF_GAIN_MIN, HIGHSHELF_GAIN_MAX, v);
		}

		void SetPeakingGain(float v) {
			mi_->gain_->value_ = Clamp(PEAKING_GAIN_MIN, PEAKING_GAIN_MAX, v);
		}

		void SetOptions(float* opts, int16_t opt_count) override {
			if (opt_count > 0) {
				SetLowShelfGain(opts[0]);
			}
			if (opt_count > 1) {
				SetPeakingGain(opts[1]);
			}
			if (opt_count > 2) {
				SetHighShelfGain(opts[2]);
			}
			if (opt_count > 3) {
				CrossFade(opts[3]);
			}
		}
	};

	float Equalizer3Band::LOWSHELF_GAIN_DEFAULT = 1.0f;
	float Equalizer3Band::LOWSHELF_GAIN_MIN = -127.0f;
	float Equalizer3Band::LOWSHELF_GAIN_MAX = 12.0f;

	float Equalizer3Band::PEAKING_GAIN_DEFAULT = -7.0f;
	float Equalizer3Band::PEAKING_GAIN_MIN = -127.0f;
	float Equalizer3Band::PEAKING_GAIN_MAX = 12.0f;

	float Equalizer3Band::HIGHSHELF_GAIN_DEFAULT = -10.0f;
	float Equalizer3Band::HIGHSHELF_GAIN_MIN = -127.0f;
	float Equalizer3Band::HIGHSHELF_GAIN_MAX = 12.0f;
}