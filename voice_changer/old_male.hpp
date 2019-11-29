#pragma once
#include "Jungle.hpp"
#include "equalizer_3_band.hpp"
#include "vibrato.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "role.hpp"
#include "effects.h"
using namespace effect;
using namespace vocaloid::node;
namespace role {
	
	class OldMale : public Role {

	private:
		BiquadNode *lowpass_;
		PhaseVocoderNode *jungle_;
		Vibrato *vibrato_;
	public:

		static float PITCH_DEFAULT;
		static float PITCH_MIN;
		static float PITCH_MAX;

		static float TEMPO_DEFAULT;
		static float TEMPO_MIN;
		static float TEMPO_MAX;

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


		explicit OldMale(BaseAudioContext *ctx) : Role(ctx) {
			id_ = Roles::OLD_MALE;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = LOWPASS_FREQ_DEFAULT;
			jungle_ = new PhaseVocoderNode(ctx);
			jungle_->pitch_ = PITCH_DEFAULT;
			jungle_->tempo_ = TEMPO_DEFAULT;
			vibrato_ = new Vibrato(ctx);
			ctx->Connect(lowpass_, jungle_);
			ctx->Connect(jungle_, vibrato_->input_);
			ctx->Connect(vibrato_->output_, gain_);
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
			if (vibrato_) {
				vibrato_->Dispose();
				delete vibrato_;
				vibrato_ = nullptr;
			}
			Role::Dispose();
		}

		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				jungle_->pitch_ = Clamp(PITCH_MIN, PITCH_MAX, options[0]);
			}
			if (option_count > 1) {
				jungle_->tempo_ = Clamp(TEMPO_MIN, TEMPO_MAX, options[1]);
			}
			if (option_count > 2) {
				lowpass_->frequency_->value_ = Clamp(LOWPASS_FREQ_MIN, LOWPASS_FREQ_MAX, options[2]);
			}
			if (option_count > 3) {
				vibrato_->SetDelay(options[3]);
			}
			if (option_count > 4) {
				vibrato_->SetDepth(options[4]);
			}
			if (option_count > 5) {
				vibrato_->SetSpeed(options[5]);
			}
			if (option_count > 6) {
				SetGain(options[6]);
			}
		}

		float TimeScale() override { return jungle_->tempo_; }
		float Delay() override { return vibrato_->Delay(); }

		AudioNode *Input() {
			return lowpass_;
		}

		void Start() override {
			vibrato_->Start();
		}

		void Resume() override {
			vibrato_->Resume();
		}
	};

	float OldMale::LOWPASS_FREQ_DEFAULT = 8000.0f;
	float OldMale::LOWPASS_FREQ_MIN = FLT_MIN;
	float OldMale::LOWPASS_FREQ_MAX = FLT_MAX;

	float OldMale::PITCH_DEFAULT = 0.8f;
	float OldMale::PITCH_MIN = 0.5f;
	float OldMale::PITCH_MAX = 2.0f;

	float OldMale::TEMPO_DEFAULT = 1.2f;
	float OldMale::TEMPO_MIN = 0.5f;
	float OldMale::TEMPO_MAX = 2.0f;

	float OldMale::VIBRATO_DELAY_DEFAULT = Vibrato::VIBRATO_DELAY_DEFAULT;
	float OldMale::VIBRATO_DELAY_MIN = Vibrato::VIBRATO_DELAY_MIN;
	float OldMale::VIBRATO_DELAY_MAX = Vibrato::VIBRATO_DELAY_MAX;

	float OldMale::VIBRATO_DEPTH_DEFAULT = Vibrato::VIBRATO_DEPTH_DEFAULT;
	float OldMale::VIBRATO_DEPTH_MIN = Vibrato::VIBRATO_DEPTH_MIN;
	float OldMale::VIBRATO_DEPTH_MAX = Vibrato::VIBRATO_DEPTH_MAX;

	float OldMale::VIBRATO_SPEED_DEFAULT = 30;
	float OldMale::VIBRATO_SPEED_MIN = Vibrato::VIBRATO_SPEED_MIN;
	float OldMale::VIBRATO_SPEED_MAX = Vibrato::VIBRATO_SPEED_MAX;
}