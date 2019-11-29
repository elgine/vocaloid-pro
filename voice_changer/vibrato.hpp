#pragma once
#include "effect.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
namespace effect {
	using namespace vocaloid::node;
	class Vibrato : public Effect {
	public:
		OscillatorNode *osc_;
		DelayNode *delay_;
		GainNode *gain_;

		static float VIBRATO_DELAY_DEFAULT;
		static float VIBRATO_DELAY_MIN;
		static float VIBRATO_DELAY_MAX;

		static float VIBRATO_DEPTH_DEFAULT;
		static float VIBRATO_DEPTH_MIN;
		static float VIBRATO_DEPTH_MAX;

		static float VIBRATO_SPEED_DEFAULT;
		static float VIBRATO_SPEED_MIN;
		static float VIBRATO_SPEED_MAX;

		explicit Vibrato(BaseAudioContext *ctx) : Effect(ctx) {
			id_ = Effects::VIBRATO;
			delay_ = new DelayNode(ctx);
			osc_ = new OscillatorNode(ctx);
			gain_ = new GainNode(ctx);
			ctx->Connect(osc_, gain_);
			ctx->Connect(gain_, delay_->delay_time_);
			ctx->Connect(input_, delay_);
			ctx->Connect(delay_, wet_);
			SetSpeed(VIBRATO_SPEED_DEFAULT);
			SetDelay(VIBRATO_DELAY_DEFAULT);
			SetDepth(VIBRATO_DEPTH_DEFAULT);
		}

		void Dispose() override {
			if (osc_) {
				osc_->Dispose();
				delete osc_;
				osc_ = nullptr;
			}
			if (delay_) {
				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;
			}
			Effect::Dispose();
		}

		float Delay() {
			return delay_->delay_time_->value_ * 0.5;
		}

		void SetDelay(float v) {
			delay_->delay_time_->value_ = Clamp(VIBRATO_DELAY_MIN, VIBRATO_DELAY_MAX, v);
		}

		void SetDepth(float v) {
			gain_->gain_->value_ = Clamp(VIBRATO_DEPTH_MIN, VIBRATO_DELAY_MAX, v);
		}

		void SetSpeed(float v) {
			osc_->SetFrequency(Clamp(VIBRATO_SPEED_MIN, VIBRATO_SPEED_MAX, v));
		}

		void Start() override {
			osc_->Start();
		}

		void Resume() override {
			osc_->Resume();
		}

		void SetOptions(float *opts, int16_t opt_count) {
			if (opt_count > 0) {
				SetSpeed(opts[0]);
			}
			if (opt_count > 1) {
				SetDelay(opts[1]);
			}
			if (opt_count > 2) {
				SetDepth(opts[2]);
			}
			if (opt_count > 3) {
				CrossFade(opts[3]);
			}
		}
	};

	float Vibrato::VIBRATO_DELAY_DEFAULT = 3.5f;
	float Vibrato::VIBRATO_DELAY_MIN = 0.5f;
	float Vibrato::VIBRATO_DELAY_MAX = 15.0f;
	float Vibrato::VIBRATO_DEPTH_DEFAULT = 0.03f;
	float Vibrato::VIBRATO_DEPTH_MIN = 0.005f;
	float Vibrato::VIBRATO_DEPTH_MAX = 0.055f;
	float Vibrato::VIBRATO_SPEED_DEFAULT = 0.002f;
	float Vibrato::VIBRATO_SPEED_MIN = 0.0005f;
	float Vibrato::VIBRATO_SPEED_MAX = 0.004f;
}