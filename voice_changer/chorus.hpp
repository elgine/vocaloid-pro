#pragma once
#include "effect.hpp"
#include "../vocaloid/wave_shaper_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid::node;
namespace effect {
	class Chorus : public Effect {
	public:
		static float CHORUS_SPEED_DEFAULT;
		static float CHORUS_SPEED_MIN;
		static float CHORUS_SPEED_MAX;

		static float CHORUS_DELAY_DEFAULT;
		static float CHORUS_DELAY_MIN;
		static float CHORUS_DELAY_MAX;

		static float CHORUS_DEPTH_DEFAULT;
		static float CHORUS_DEPTH_MIN;
		static float CHORUS_DEPTH_MAX;

		DelayNode *delay_;
		GainNode *gain_;
		OscillatorNode *osc_;

		explicit Chorus(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::CHORUS;
			delay_ = new DelayNode(ctx);
			osc_ = new OscillatorNode(ctx);
			gain_ = new GainNode(ctx);

			ctx->Connect(osc_, gain_);
			ctx->Connect(gain_, delay_->delay_time_);
			ctx->Connect(input_, wet_);
			ctx->Connect(input_, delay_);
			ctx->Connect(delay_, wet_);

			SetSpeed(CHORUS_SPEED_DEFAULT);
			SetDepth(CHORUS_DEPTH_DEFAULT);
			SetDelay(CHORUS_DELAY_DEFAULT);
		}

		void Dispose() override {
			delay_->Dispose();
			delete delay_;
			delay_ = nullptr;
			delete gain_;
			gain_ = nullptr;
			delete osc_;
			osc_ = nullptr;
		}

		void Start() override {
			osc_->Start();
		}

		void Resume() override {
			osc_->Resume();
		}

		void Stop() override {
			osc_->Stop();
		}

		void SetSpeed(float v) {
			v = Clamp(CHORUS_SPEED_MIN, CHORUS_SPEED_MAX, v);
			osc_->SetFrequency(v);
		}

		void SetDelay(float v) {
			v = Clamp(CHORUS_DELAY_MIN, CHORUS_DELAY_MAX, v);
			delay_->delay_time_->value_ = v;
		}

		void SetDepth(float v) {
			v = Clamp(CHORUS_DEPTH_MIN, CHORUS_DEPTH_MAX, v);
			gain_->gain_->value_ = v;
		}

		void SetOptions(float* opts, int16_t opt_count) override {
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

	float Chorus::CHORUS_SPEED_DEFAULT = 3.5f;
	float Chorus::CHORUS_SPEED_MIN = 0.5f;
	float Chorus::CHORUS_SPEED_MAX = 15.0f;
	float Chorus::CHORUS_DELAY_DEFAULT = 0.03f;
	float Chorus::CHORUS_DELAY_MIN = 0.005f;
	float Chorus::CHORUS_DELAY_MAX = 0.055f;
	float Chorus::CHORUS_DEPTH_DEFAULT = 0.002f;
	float Chorus::CHORUS_DEPTH_MIN = 0.0005f;
	float Chorus::CHORUS_DEPTH_MAX = 0.004f;
}