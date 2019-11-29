#pragma once
#include "role.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
using namespace vocaloid;
namespace role {
	class Alien : public Role {
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

		explicit Alien(BaseAudioContext *ctx) :Role(ctx) {
			id_ = Roles::ALIEN;
			delay_ = new DelayNode(ctx);
			osc_ = new OscillatorNode(ctx);
			osc_gain_ = new GainNode(ctx);
			ctx->Connect(osc_, osc_gain_);
			ctx->Connect(osc_gain_, delay_->delay_time_);
			ctx->Connect(delay_, gain_);

			osc_->SetFrequency(LFO_FREQ_DEFAULT);
			osc_gain_->gain_->value_ = LFO_GAIN_DEFAULT;
			delay_->delay_time_->value_ = 0.05f;
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
			Role::Dispose();
		}

		// [lfo_freq, lfo_gain, gain]
		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				SetLFOFrequency(options[0]);
			}
			if (option_count > 1) {
				SetLFOGain(options[1]);
			}
			if (option_count > 2) {
				SetGain(options[2]);
			}
		}

		void SetLFOFrequency(float v) {
			osc_->SetFrequency(Clamp(LFO_FREQ_MIN, LFO_FREQ_MAX, v));
		}

		void SetLFOGain(float v) {
			osc_gain_->gain_->value_ = Clamp(LFO_GAIN_MIN, LFO_GAIN_MAX, v);
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

	float Alien::LFO_FREQ_DEFAULT = 5.0f;
	float Alien::LFO_FREQ_MIN = -22050;
	float Alien::LFO_FREQ_MAX = 22050;

	float Alien::LFO_GAIN_DEFAULT = 0.05f;
	float Alien::LFO_GAIN_MIN = 0.0;
	float Alien::LFO_GAIN_MAX = 4.0f;
}