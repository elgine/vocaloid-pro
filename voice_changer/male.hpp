#pragma once
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/jungle.hpp"
#include "effect.hpp"
#include "effects.h"
using namespace vocaloid::composite;

namespace effect {
	class Male : public Effect {
	private:
		BiquadNode *lowpass_;
		Jungle *jungle_;
		//PhaseVocoderNode *jungle_;
	public:

		static float LOWPASS_FREQ_DEFAULT;
		static float LOWPASS_FREQ_MIN;
		static float LOWPASS_FREQ_MAX;

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;

		explicit Male(BaseAudioContext* ctx) :Effect(ctx) {
			id_ = Effects::MALE;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = LOWPASS_FREQ_DEFAULT;
			/*jungle_ = new PhaseVocoderNode(ctx);
			jungle_->pitch_ = 0.9f;
			ctx->Connect(lowpass_, jungle_);
			ctx->Connect(jungle_, gain_);*/
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(PITCH_OFFSET_DEFAULT);

			ctx->Connect(lowpass_, jungle_->input_);
			ctx->Connect(jungle_->output_, gain_);
		}

		void Dispose() override {
			if (lowpass_) {
				lowpass_->Dispose();
				delete lowpass_;
				lowpass_ = nullptr;
			}
			if (jungle_) {
				jungle_->Dispose();
				delete jungle_;
				jungle_ = nullptr;
			}
			Effect::Dispose();
		}

		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				lowpass_->frequency_->value_ = Clamp(LOWPASS_FREQ_MIN, LOWPASS_FREQ_MAX, options[0]);
			}
			if (option_count > 1) {
				//jungle_->pitch_ = 1.0f + Clamp(PITCH_OFFSET_MIN, PITCH_OFFSET_MAX, options[1]);
				jungle_->SetPitchOffset(Clamp(PITCH_OFFSET_MIN, PITCH_OFFSET_MAX, options[1]));
			}
			if (option_count > 2) {
				SetGain(options[2]);
			}
		}

		AudioNode *Input() {
			return lowpass_;
		}

		void Start() override {
			jungle_->Start();
		}

		void Resume() override {
			jungle_->Resume();
		}
	};

	float Male::LOWPASS_FREQ_DEFAULT = 4000.0f;
	float Male::LOWPASS_FREQ_MIN = 2000.0f;
	float Male::LOWPASS_FREQ_MAX = 20000.0f;

	float Male::PITCH_OFFSET_DEFAULT = -0.1f;
	float Male::PITCH_OFFSET_MIN = -0.5f;
	float Male::PITCH_OFFSET_MAX = 2.0f;
}