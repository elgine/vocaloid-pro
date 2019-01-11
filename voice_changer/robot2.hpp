#pragma once
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "effects.h"
#include "effect.hpp"

namespace effect {
	
	class Robot2 : public Effect {

	private:
		OscillatorNode *osc1_;
		OscillatorNode *osc2_;
		OscillatorNode *osc3_;
		GainNode *osc_gain_;
		DelayNode *delay_;
	public:

		explicit Robot2(BaseAudioContext* ctx):Effect(ctx) {
			id_ = Effects::ROBOT2;
			osc1_ = new OscillatorNode(ctx);
			osc2_ = new OscillatorNode(ctx);
			osc3_ = new OscillatorNode(ctx);
			osc_gain_ = new GainNode(ctx);
			delay_ = new DelayNode(ctx);

			osc1_->SetWaveform(50, vocaloid::dsp::WAVEFORM_TYPE::SAWTOOTH);
			osc2_->SetWaveform(1000, vocaloid::dsp::WAVEFORM_TYPE::SAWTOOTH);
			osc3_->SetFrequency(50);
			delay_->delay_time_->value_ = 0.01f;
			osc_gain_->gain_->value_ = 0.004f;

			ctx_->Connect(osc1_, osc_gain_);
			ctx_->Connect(osc2_, osc_gain_);
			ctx_->Connect(osc3_, osc_gain_);
			ctx_->Connect(osc_gain_, delay_->delay_time_);
			ctx_->Connect(delay_, gain_);
		}

		void Dispose() override {
			if (osc1_) {
				osc1_->Dispose();
				delete osc1_;
				osc1_ = nullptr;
			}
			if (osc2_) {
				osc2_->Dispose();
				delete osc2_;
				osc2_ = nullptr;
			}
			if (osc3_) {
				osc3_->Dispose();
				delete osc3_;
				osc3_ = nullptr;
			}
			if (osc_gain_) {
				osc_gain_->Dispose();
				delete osc_gain_;
				osc_gain_ = nullptr;
			}
			if (delay_) {
				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;
			}
			Effect::Dispose();
		}

		void Start() override {
			osc1_->Start();
			osc2_->Start();
			osc3_->Start();
		}

		void Resume() override {
			osc1_->Resume();
			osc2_->Resume();
			osc3_->Resume();
		}

		AudioNode* Input() {
			return delay_;
		}
	};
}