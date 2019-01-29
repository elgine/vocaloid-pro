#pragma once
#include "../vocaloid/vibrato.hpp"
#include "../vocaloid/jungle.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "effect.hpp"
#include "effects.h"
using namespace vocaloid::composite;
using namespace vocaloid::node;
namespace effect {
	
	class OldFemale : public Effect {
	private:
		BiquadNode *highpass_;
		PhaseVocoderNode *jungle_;
		composite::Vibrato *vibrato_;
	public:

		static float HIGHPASS_FREQ_DEFAULT;
		static float HIGHPASS_FREQ_MIN;
		static float HIGHPASS_FREQ_MAX;

		static float PITCH_DEFAULT;
		static float PITCH_MIN;
		static float PITCH_MAX;

		static float TEMPO_DEFAULT;
		static float TEMPO_MIN;
		static float TEMPO_MAX;

		static float VIBRATO_DELAY_DEFAULT;
		static float VIBRATO_DELAY_MIN;
		static float VIBRATO_DELAY_MAX;

		static float VIBRATO_DEPTH_DEFAULT;
		static float VIBRATO_DEPTH_MIN;
		static float VIBRATO_DEPTH_MAX;

		static float VIBRATO_SPEED_DEFAULT;
		static float VIBRATO_SPEED_MIN;
		static float VIBRATO_SPEED_MAX;

		explicit OldFemale(BaseAudioContext *ctx) : Effect(ctx) {
			id_ = Effects::OLD_FEMALE;
			highpass_ = new BiquadNode(ctx);
			highpass_->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
			highpass_->frequency_->value_ = HIGHPASS_FREQ_DEFAULT;
			jungle_ = new PhaseVocoderNode(ctx);
			jungle_->pitch_ = PITCH_DEFAULT;
			jungle_->tempo_ = TEMPO_DEFAULT;
			vibrato_ = new composite::Vibrato(ctx);
			vibrato_->SetOptions({
				VIBRATO_DELAY_DEFAULT,
				VIBRATO_DEPTH_DEFAULT,
				VIBRATO_SPEED_DEFAULT,
			});
			ctx->Connect(jungle_, highpass_);
			ctx->Connect(highpass_, vibrato_->input_);
			ctx->Connect(vibrato_->output_, gain_);
		}

		float TimeScale() override { return jungle_->tempo_; }
		float Delay() override { return vibrato_->Delay(); }

		void Dispose() override {
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
			if (highpass_) {
				highpass_->Dispose();
				delete highpass_;
				highpass_ = nullptr;
			}
			Effect::Dispose();
		}
		
		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				highpass_->frequency_->value_ = Clamp(HIGHPASS_FREQ_MIN, HIGHPASS_FREQ_MAX, options[0]);
			}
			if (option_count > 1) {
				jungle_->pitch_ = Clamp(PITCH_MIN, PITCH_MAX, options[1]);
			}
			if (option_count > 2) {
				jungle_->tempo_ = Clamp(TEMPO_MIN, TEMPO_MAX, options[2]);
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
				SetGain(options[5]);
			}
		}

		AudioNode *Input() {
			return jungle_;
		}

		void Start() override {
			vibrato_->Start();
		}

		void Resume() override {
			vibrato_->Resume();
		}
	};

	float OldFemale::HIGHPASS_FREQ_DEFAULT = 120.0f;
	float OldFemale::HIGHPASS_FREQ_MIN = FLT_MIN;
	float OldFemale::HIGHPASS_FREQ_MAX = FLT_MAX;

	float OldFemale::PITCH_DEFAULT = 1.3f;
	float OldFemale::PITCH_MIN = 0.5f;
	float OldFemale::PITCH_MAX = 2.0f;

	float OldFemale::TEMPO_DEFAULT = 1.2f;
	float OldFemale::TEMPO_MIN = 0.5f;
	float OldFemale::TEMPO_MAX = 2.0f;

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