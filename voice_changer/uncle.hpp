#pragma once
#include "../vocaloid/maths.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "effects.h"
#include "effect.hpp"
using namespace vocaloid;
namespace effect {
	class Uncle : public Effect {
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

		// [lowpass_freq, pitch, gain]
		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				SetLowpassFrequency(options[0]);
			}
			if (option_count > 1) {
				SetPitch(options[1]);
			}
			if (option_count > 2) {
				SetGain(options[2]);
			}
		}

		explicit Uncle(AudioContext *ctx): Effect(ctx) {
			id_ = Effects::UNCLE;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = LOWPASS_FREQ_DEFAULT;
			phase_vocoder_ = new PhaseVocoderNode(ctx);
			phase_vocoder_->pitch_ = PITCH_DEFAULT;

			ctx->Connect(lowpass_, phase_vocoder_);
			ctx->Connect(phase_vocoder_, gain_);
		}

		void SetLowpassFrequency(float freq) {
			lowpass_->frequency_->value_ = Clamp(LOWPASS_FREQ_MIN, LOWPASS_FREQ_MAX, freq);
		}

		void SetPitch(float v) {
			phase_vocoder_->pitch_ = Clamp(PITCH_MIN, PITCH_MAX, v);
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

	float Uncle::LOWPASS_FREQ_DEFAULT = 4500;
	float Uncle::LOWPASS_FREQ_MIN = 500;
	float Uncle::LOWPASS_FREQ_MAX = 20000;

	float Uncle::PITCH_DEFAULT = 0.7f;
	float Uncle::PITCH_MIN = 0.5f;
	float Uncle::PITCH_MAX = 1.0f;
}