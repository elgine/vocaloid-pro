#pragma once
#include "../vocaloid/vibrato.hpp"
#include "../vocaloid/jungle.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
#include "effect.hpp"
#include "effects.h"
namespace effect {
	
	class OldFemale : public Effect {
	private:
		BiquadNode *highpass_;
		Jungle *jungle_;
		composite::Vibrato *vibrato_;
	public:

		static float HIGHPASS_FREQ_DEFAULT;
		static float HIGHPASS_FREQ_MIN;
		static float HIGHPASS_FREQ_MAX;

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;

		static float VIBRATO_DELAY_DEFAULT;
		static float VIBRATO_DELAY_MIN;
		static float VIBRATO_DELAY_MAX;

		static float VIBRATO_DEPTH_DEFAULT;
		static float VIBRATO_DEPTH_MIN;
		static float VIBRATO_DEPTH_MAX;

		static float VIBRATO_SPEED_DEFAULT;
		static float VIBRATO_SPEED_MIN;
		static float VIBRATO_SPEED_MAX;

		explicit OldFemale(AudioContext *ctx) : Effect(ctx) {
			id_ = Effects::OLD_FEMALE;
			highpass_ = new BiquadNode(ctx);
			highpass_->type_ = BIQUAD_TYPE::HIGH_PASS;
			highpass_->frequency_->value_ = 1250;
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(0.25f);
			vibrato_ = new composite::Vibrato(ctx);
			vibrato_->SetOptions({
				0.05f,
				0.001f,
				7.2f,
			});

			ctx->Connect(jungle_->output_, highpass_);
			ctx->Connect(highpass_, vibrato_->input_);
			ctx->Connect(vibrato_->output_, gain_);
		}

		AudioNode *Input() {
			return jungle_->input_;
		}

		void Start() override {
			jungle_->Start();
			vibrato_->Start();
		}

		void Dispose() override {
			jungle_->Dispose();
			delete jungle_;
			jungle_ = nullptr;

			vibrato_->Dispose();
			delete vibrato_;
			vibrato_ = nullptr;

			highpass_->Dispose();
			delete highpass_;
			highpass_ = nullptr;

			Effect::Dispose();
		}
	};

	float OldFemale::HIGHPASS_FREQ_DEFAULT = 1250.0f;
	float OldFemale::HIGHPASS_FREQ_MIN = 500.0f;
	float OldFemale::HIGHPASS_FREQ_MAX = 8000.0f;

	float OldFemale::PITCH_OFFSET_DEFAULT = 0.25f;
	float OldFemale::PITCH_OFFSET_MIN = 0.0f;
	float OldFemale::PITCH_OFFSET_MAX = 0.5f;

	float OldFemale::VIBRATO_DELAY_DEFAULT = Vibrato::VIBRATO_DELAY_DEFAULT;
	float OldFemale::VIBRATO_DELAY_MIN = Vibrato::VIBRATO_DELAY_MIN;
	float OldFemale::VIBRATO_DELAY_MAX = Vibrato::VIBRATO_DELAY_MAX;

	float OldFemale::VIBRATO_DEPTH_DEFAULT = Vibrato::VIBRATO_DEPTH_DEFAULT;
	float OldFemale::VIBRATO_DEPTH_MIN = Vibrato::VIBRATO_DEPTH_MIN;
	float OldFemale::VIBRATO_DEPTH_MAX = Vibrato::VIBRATO_DEPTH_MAX;

	float OldFemale::VIBRATO_SPEED_DEFAULT = Vibrato::VIBRATO_SPEED_DEFAULT;
	float OldFemale::VIBRATO_SPEED_MIN = Vibrato::VIBRATO_SPEED_MIN;
	float OldFemale::VIBRATO_SPEED_MAX = Vibrato::VIBRATO_SPEED_MAX;
}