#pragma once
#include "effect.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "effects.h"
namespace effect {

	class Alien : public Effect {
	private:
		DelayNode *delay_;
		OscillatorNode *osc_;
		GainNode *osc_gain_;
	public:

		static float LFO_FREQ_DEFAULT;
		static float LFO_FREQ_MIN;
		static float LFO_FREQ_MAX;

		static float LFO_GAIN_DEFAULT;
		static float LFO_GAIN_MIN;
		static float LFO_GAIN_MAX;

		explicit Alien(AudioContext *ctx) :Effect(ctx) {
			id_ = Effects::ALIEN;
			delay_ = new DelayNode(ctx);
			osc_ = new OscillatorNode(ctx);
			osc_gain_ = new GainNode(ctx);
			ctx->Connect(osc_, osc_gain_);
			ctx->Connect(osc_gain_, delay_->delay_time_);
			ctx->Connect(delay_, gain_);

			osc_->SetFrequency(5.0f);
			osc_gain_->gain_->value_ = 0.05f;
			delay_->delay_time_->value_ = 0.05f;
		}

		void SetLFOFrequency(float v) {
			osc_->SetFrequency(v);
		}

		void SetLFOGain(float v) {
			osc_gain_->gain_->value_ = v;
		}

		void Start() override {
			osc_->Start();
		}

		AudioNode* Input() override {
			return delay_;
		}

		void Dispose() override {
			if (delay_ != nullptr) {
				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;
			}
			if (osc_ != nullptr) {
				osc_->Dispose();
				delete osc_;
				osc_ = nullptr;
			}
			if (osc_gain_ != nullptr) {
				osc_gain_->Dispose();
				delete osc_;
				osc_gain_ = nullptr;
			}
			Effect::Dispose();
		}
	};

	float Alien::LFO_FREQ_DEFAULT = 5.0f;
	float Alien::LFO_FREQ_MIN = 20.0f;
	float Alien::LFO_FREQ_MAX = 40.0f;

	float Alien::LFO_GAIN_DEFAULT = 0.05f;
	float Alien::LFO_GAIN_MIN = 0.01f;
	float Alien::LFO_GAIN_MAX = 0.05f;
}