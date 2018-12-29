#pragma once
#include "../vocaloid/audio_context.hpp"
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

		explicit Robot1(AudioContext *ctx):Effect(ctx) {
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

		void Start() override {
			osc_->Start();
		}

		AudioNode* Input() override {
			return delay_;
		}

		void Dispose() override {
			delay_->Dispose();
			delete delay_;
			delay_ = nullptr;

			osc_->Dispose();
			delete osc_;
			osc_ = nullptr;

			osc_gain_->Dispose();
			delete osc_gain_;
			osc_gain_ = nullptr;

			biquad_->Dispose();
			delete biquad_;
			biquad_ = nullptr;
			Effect::Dispose();
		}
	};
}
