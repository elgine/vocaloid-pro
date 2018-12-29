#pragma once
#include "../vocaloid/Jungle.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
#include "../vocaloid/vibrato.hpp"
#include "effect.hpp"
#include "effects.h"
using namespace vocaloid::composite;
namespace effect {
	
	class OldMale : public Effect {

	private:
		BiquadNode *lowpass_;
		Jungle *jungle_;
		Vibrato *vibrato_;
	public:

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;

		static float LOWPASS_FREQ_DEFAULT;
		static float LOWPASS_FREQ_MIN;
		static float LOWPASS_FREQ_MAX;

		static float VIBRATO_DELAY_DEFAULT;
		static float VIBRATO_DELAY_MIN;
		static float VIBRATO_DELAY_MAX;

		static float VIBRATO_DEPTH_DEFAULT;
		static float VIBRATO_DEPTH_MIN;
		static float VIBRATO_DEPTH_MAX;

		static float VIBRATO_SPEED_DEFAULT;
		static float VIBRATO_SPEED_MIN;
		static float VIBRATO_SPEED_MAX;


		explicit OldMale(AudioContext *ctx) : Effect(ctx) {
			id_ = Effects::OLD_MALE;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = 2000;
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(-0.2);
			vibrato_ = new Vibrato(ctx);
			vibrato_->SetOptions({
				0.03f,
				0.002f,
				5.0f,
			});

			ctx->Connect(lowpass_, jungle_->input_);
			ctx->Connect(jungle_->output_, vibrato_->input_);
			ctx->Connect(vibrato_->output_, gain_);
		}

		AudioNode *Input() {
			return lowpass_;
		}

		void Start() override {
			jungle_->Start();
			vibrato_->Start();
		}

		void Dispose() override {
			lowpass_->Dispose();
			delete lowpass_;
			lowpass_ = nullptr;

			jungle_->Dispose();
			delete jungle_;
			jungle_ = nullptr;

			vibrato_->Dispose();
			delete vibrato_;
			vibrato_ = nullptr;

			Effect::Dispose();
		}
	};

	float OldMale::LOWPASS_FREQ_DEFAULT = 2000.0f;
	float OldMale::LOWPASS_FREQ_MIN = 500.0f;
	float OldMale::LOWPASS_FREQ_MAX = 4000.0f;

	float OldMale::PITCH_OFFSET_DEFAULT = -0.2f;
	float OldMale::PITCH_OFFSET_MIN = -0.8f;
	float OldMale::PITCH_OFFSET_MAX = 0.0f;

	float OldMale::VIBRATO_DELAY_DEFAULT = Vibrato::VIBRATO_DELAY_DEFAULT;
	float OldMale::VIBRATO_DELAY_MIN = Vibrato::VIBRATO_DELAY_MIN;
	float OldMale::VIBRATO_DELAY_MAX = Vibrato::VIBRATO_DELAY_MAX;

	float OldMale::VIBRATO_DEPTH_DEFAULT = Vibrato::VIBRATO_DEPTH_DEFAULT;
	float OldMale::VIBRATO_DEPTH_MIN = Vibrato::VIBRATO_DEPTH_MIN;
	float OldMale::VIBRATO_DEPTH_MAX = Vibrato::VIBRATO_DEPTH_MAX;

	float OldMale::VIBRATO_SPEED_DEFAULT = Vibrato::VIBRATO_SPEED_DEFAULT;
	float OldMale::VIBRATO_SPEED_MIN = Vibrato::VIBRATO_SPEED_MIN;
	float OldMale::VIBRATO_SPEED_MAX = Vibrato::VIBRATO_SPEED_MAX;
}