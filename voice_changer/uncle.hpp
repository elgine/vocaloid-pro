#pragma once
#include "../vocaloid/maths.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/jungle.hpp"
#include "effects.h"
#include "effect.hpp"
using namespace vocaloid;
using namespace vocaloid::composite;
namespace effect {
	class Uncle : public Effect {
	private:
		BiquadNode *lowpass_;
		Jungle *jungle_;
	public:

		static float LOWPASS_FREQ_DEFAULT;
		static float LOWPASS_FREQ_MIN;
		static float LOWPASS_FREQ_MAX;

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;

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
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(PITCH_OFFSET_DEFAULT);

			ctx->Connect(lowpass_, jungle_->input_);
			ctx->Connect(jungle_->output_, gain_);
		}

		void SetLowpassFrequency(float freq) {
			lowpass_->frequency_->value_ = Clamp(LOWPASS_FREQ_MIN, LOWPASS_FREQ_MAX, freq);
		}

		void SetPitch(float v) {
			jungle_->SetPitchOffset(Clamp(PITCH_OFFSET_MIN, PITCH_OFFSET_MAX, v));
		}

		void Dispose() override {
			lowpass_->Dispose();
			delete lowpass_;
			lowpass_ = nullptr;

			jungle_->Dispose();
			delete jungle_;
			jungle_ = nullptr;

			Effect::Dispose();
		}

		AudioNode *Input() {
			return lowpass_;
		}
	};

	float Uncle::LOWPASS_FREQ_DEFAULT = 4500;
	float Uncle::LOWPASS_FREQ_MIN = 500;
	float Uncle::LOWPASS_FREQ_MAX = 20000;

	float Uncle::PITCH_OFFSET_DEFAULT = -0.3f;
	float Uncle::PITCH_OFFSET_MIN = -1.0f;
	float Uncle::PITCH_OFFSET_MAX = 0.0f;
}