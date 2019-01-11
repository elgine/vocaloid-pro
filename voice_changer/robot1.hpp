#pragma once
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "effects.h"
#include "effect.hpp"

namespace effect {

	class Robot1 : public Effect {
	private:
		OscillatorNode *osc_;
		DelayNode *delay_;
		GainNode *osc_gain_;
		BiquadNode *biquad_;

	public:

		explicit Robot1(BaseAudioContext *ctx):Effect(ctx) {
			id_ = Effects::ROBOT1;
			delay_ = new DelayNode(ctx, 0.01);
			osc_ = new OscillatorNode(ctx);
			osc_->SetFrequency(700.0f);
			osc_gain_ = new GainNode(ctx, 0.004f);
			biquad_ = new BiquadNode(ctx);
			biquad_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			biquad_->frequency_->value_ = 695;
			ctx_->Connect(osc_, osc_gain_);
			ctx_->Connect(osc_gain_, delay_->delay_time_);
			ctx_->Connect(delay_, biquad_);
			ctx_->Connect(biquad_, gain_);
		}

		void Dispose() override {
			if (delay_) {
				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;
			}
			if (osc_) {
				osc_->Dispose();
				delete osc_;
				osc_ = nullptr;
			}
			if (osc_gain_) {
				osc_gain_->Dispose();
				delete osc_gain_;
				osc_gain_ = nullptr;
			}
			if (biquad_) {
				biquad_->Dispose();
				delete biquad_;
				biquad_ = nullptr;
			}
			Effect::Dispose();
		}

		void Start() override {
			osc_->Start();
		}

		void Resume() override {
			osc_->Resume();
		}

		AudioNode* Input() override {
			return delay_;
		}
	};
}
