#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "effects.h"
#include "effect.hpp"

namespace effect {
	class BigGuy : public Effect {
	private:
		BiquadNode *lowpass_;
		PhaseVocoderNode *phase_vocoder_;
	public:

		static float LOWPASS_FREQ_DEFAULT;
		static float LOWPASS_FREQ_MIN;
		static float LOWPASS_FREQ_MAX;

		static float PITCH_DEFAULT;
		static float PITCH_MIN;
		static float PITCH_MAX;

		explicit BigGuy(AudioContext *ctx): Effect(ctx) {
			id_ = Effects::BIG_GUY;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = 2000;
			phase_vocoder_ = new PhaseVocoderNode(ctx);
			phase_vocoder_->pitch_ = 0.88f;

			ctx->Connect(lowpass_, phase_vocoder_);
			ctx->Connect(phase_vocoder_, gain_);
		}

		void SetLowpassFrequency(float freq) {
			lowpass_->frequency_->value_ = freq;
		}

		void SetPitch(float v) {
			phase_vocoder_->pitch_ = v;
		}

		void Dispose() override {
			lowpass_->Dispose();
			delete lowpass_;
			lowpass_ = nullptr;

			phase_vocoder_->Dispose();
			delete phase_vocoder_;
			phase_vocoder_ = nullptr;

			Effect::Dispose();
		}

		AudioNode *Input() {
			return lowpass_;
		}
	};

	float BigGuy::LOWPASS_FREQ_DEFAULT = 2000;
	float BigGuy::LOWPASS_FREQ_MIN = 500;
	float BigGuy::LOWPASS_FREQ_MAX = 4000;

	float BigGuy::PITCH_DEFAULT = 0.88;
	float BigGuy::PITCH_MIN = 0.5f;
	float BigGuy::PITCH_MAX = 1.0f;
}