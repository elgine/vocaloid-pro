#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
#include "effect.hpp"
#include "effects.h"
using namespace vocaloid::composite;
namespace effect {
	
	class Child : public Effect {
	private:
		PhaseVocoderNode* phase_vocoder_;
		Equalizer3Band* equalizer_;
		BiquadNode* highpass_;
	public:

		static float PITCH_DEFAULT;
		static float PITCH_MIN;
		static float PITCH_MAX;

		static float LOWSHELF_GAIN_DEFAULT;
		static float LOWSHELF_GAIN_MIN;
		static float LOWSHELF_GAIN_MAX;

		static float PEAKING_GAIN_DEFAULT;
		static float PEAKING_GAIN_MIN;
		static float PEAKING_GAIN_MAX;

		static float HIGHSHELF_GAIN_DEFAULT;
		static float HIGHSHELF_GAIN_MIN;
		static float HIGHSHELF_GAIN_MAX;

		static float HIGHPASS_DEFAULT;
		static float HIGHPASS_MIN;
		static float HIGHPASS_MAX;

		// [pitch, lowshelf_gain, peaking_gain, highshelf_gain, highpass]
		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				highpass_->frequency_->value_ = Clamp(HIGHPASS_MIN, HIGHPASS_MAX, options[0]);
			}
			if (option_count > 1) {
				equalizer_->SetLowShelfGain(options[1]);
			}
			if (option_count > 2) {
				equalizer_->SetPeakingGain(options[2]);
			}
			if (option_count > 3) {
				equalizer_->SetHighShelfGain(options[3]);
			}
			if (option_count > 4) {
				phase_vocoder_->pitch_ = Clamp(PITCH_MIN, PITCH_MAX, options[0]);
			}
			if (option_count > 5) {
				SetGain(options[4]);
			}
		}

		explicit Child(BaseAudioContext *ctx): Effect(ctx) {
			id_ = Effects::CHILD;
			phase_vocoder_ = new PhaseVocoderNode(ctx);
			phase_vocoder_->pitch_ = PITCH_DEFAULT;
			equalizer_ = new Equalizer3Band(ctx);
			highpass_ = new BiquadNode(ctx);
			highpass_->frequency_->value_ = HIGHPASS_DEFAULT;
			highpass_->type_ = BIQUAD_TYPE::HIGH_PASS;

			ctx->Connect(equalizer_->output_, phase_vocoder_);
			ctx->Connect(phase_vocoder_, highpass_);
			ctx->Connect(highpass_, gain_);
		}

		void Dispose() override {
			if (phase_vocoder_) {
				phase_vocoder_->Dispose();
				delete phase_vocoder_;
				phase_vocoder_ = nullptr;
			}
			if (equalizer_) {
				equalizer_->Dispose();
				delete equalizer_;
				equalizer_ = nullptr;
			}
			if (highpass_) {
				highpass_->Dispose();
				delete highpass_;
				highpass_ = nullptr;
			}
			Effect::Dispose();
		}

		AudioNode* Input() {
			return equalizer_->input_;
		}
	};

	float Child::HIGHPASS_DEFAULT = 50.0f;
	float Child::HIGHPASS_MIN = FLT_MIN;
	float Child::HIGHPASS_MAX = FLT_MAX;

	float Child::LOWSHELF_GAIN_DEFAULT = Equalizer3Band::LOWSHELF_GAIN_DEFAULT;
	float Child::LOWSHELF_GAIN_MIN = Equalizer3Band::LOWSHELF_GAIN_MIN;
	float Child::LOWSHELF_GAIN_MAX = Equalizer3Band::LOWSHELF_GAIN_MAX;

	float Child::PEAKING_GAIN_DEFAULT = Equalizer3Band::PEAKING_GAIN_DEFAULT;
	float Child::PEAKING_GAIN_MIN = Equalizer3Band::PEAKING_GAIN_MIN;
	float Child::PEAKING_GAIN_MAX = Equalizer3Band::PEAKING_GAIN_MAX;

	float Child::HIGHSHELF_GAIN_DEFAULT = Equalizer3Band::HIGHSHELF_GAIN_DEFAULT;
	float Child::HIGHSHELF_GAIN_MIN = Equalizer3Band::HIGHSHELF_GAIN_MIN;
	float Child::HIGHSHELF_GAIN_MAX = Equalizer3Band::HIGHSHELF_GAIN_MAX;

	float Child::PITCH_DEFAULT = 1.5f;
	float Child::PITCH_MIN = 0.5f;
	float Child::PITCH_MAX = 2.0f;
}