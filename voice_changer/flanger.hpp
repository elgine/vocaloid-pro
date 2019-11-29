#pragma once
#include "effect.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
namespace effect {
	using namespace vocaloid::node;

	class Flanger : public Effect {
	public:
		OscillatorNode *osc_;
		DelayNode *delay_;
		GainNode *gain_;
		GainNode *feedback_;

		static float FLANGER_SPEED_DEFAULT;
		static float FLANGER_SPEED_MIN;
		static float FLANGER_SPEED_MAX;

		static float FLANGER_DELAY_DEFAULT;
		static float FLANGER_DELAY_MIN;
		static float FLANGER_DELAY_MAX;

		static float FLANGER_DEPTH_DEFAULT;
		static float FLANGER_DEPTH_MIN;
		static float FLANGER_DEPTH_MAX;

		static float FLANGER_FEEDBACK_DEFAULT;
		static float FLANGER_FEEDBACK_MIN;
		static float FLANGER_FEEDBACK_MAX;

		explicit Flanger(BaseAudioContext *ctx):Effect(ctx) {
			id_ = Effects::FLANGER;
			osc_ = new OscillatorNode(ctx);
			delay_ = new DelayNode(ctx);
			gain_ = new GainNode(ctx);
			feedback_ = new GainNode(ctx);

			ctx->Connect(osc_, gain_);
			ctx->Connect(gain_, delay_->delay_time_);
			ctx->Connect(input_, wet_);
			ctx->Connect(input_, delay_);
			ctx->Connect(delay_, wet_);
			ctx->Connect(delay_, feedback_);
			ctx->Connect(feedback_, input_);

			SetSpeed(FLANGER_SPEED_DEFAULT);
			SetDepth(FLANGER_DEPTH_DEFAULT);
			SetFeedback(FLANGER_FEEDBACK_DEFAULT);
			SetDelay(FLANGER_DELAY_DEFAULT);
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
			if (gain_) {
				gain_->Dispose();
				delete gain_;
				gain_ = nullptr;
			}
			if (feedback_) {
				feedback_->Dispose();
				delete feedback_;
				feedback_ = nullptr;
			}
			Effect::Dispose();
		}

		void Start() override {
			osc_->Start();
		}

		void SetSpeed(float v) {
			v = Clamp(FLANGER_SPEED_MIN, FLANGER_SPEED_MAX, v);
			osc_->SetFrequency(v);
		}

		void SetFeedback(float v) {
			v = Clamp(FLANGER_FEEDBACK_MIN, FLANGER_FEEDBACK_MAX, v);
			feedback_->gain_->value_ = v;
		}

		void SetDelay(float v) {
			v = Clamp(FLANGER_DELAY_MIN, FLANGER_DELAY_MAX, v);
			delay_->delay_time_->value_ = v;
		}

		void SetDepth(float v) {
			v = Clamp(FLANGER_DEPTH_MIN, FLANGER_DEPTH_MAX, v);
			gain_->gain_->value_ = v;
		}

		void SetOptions(float *opts, int16_t opt_count) override {
			if (opt_count > 0) {
				SetSpeed(opts[0]);
			}
			if (opt_count > 1) {
				SetFeedback(opts[1]);
			}
			if (opt_count > 2) {
				SetDelay(opts[2]);
			}
			if (opt_count > 3) {
				SetDepth(opts[3]);
			}
			if (opt_count > 4) {
				CrossFade(opts[4]);
			}
		}
	};

	float Flanger::FLANGER_SPEED_DEFAULT = 0.25f;
	float Flanger::FLANGER_SPEED_MIN = 0.05f;
	float Flanger::FLANGER_SPEED_MAX = 5.0f;
	float Flanger::FLANGER_DELAY_DEFAULT = 0.005f;
	float Flanger::FLANGER_DELAY_MIN = 0.001f;
	float Flanger::FLANGER_DELAY_MAX = 0.02f;
	float Flanger::FLANGER_DEPTH_DEFAULT = 0.002f;
	float Flanger::FLANGER_DEPTH_MIN = 0.0005f;
	float Flanger::FLANGER_DEPTH_MAX = 0.005f;
	float Flanger::FLANGER_FEEDBACK_DEFAULT = 0.5f;
	float Flanger::FLANGER_FEEDBACK_MIN = 0.0f;
	float Flanger::FLANGER_FEEDBACK_MAX = 1.0f;
}