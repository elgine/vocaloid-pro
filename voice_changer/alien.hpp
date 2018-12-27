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

		void Start() override {
			osc_->Start();
		}

		AudioNode* Input() override {
			return delay_;
		}

		void SetFrequency(float freq) {
			ctx_->Lock();
			osc_->SetFrequency(freq);
			ctx_->Unlock();
		}

		void SetDelay(float delay) {
			ctx_->Lock();
			delay_->delay_time_->value_ = delay;
			ctx_->Unlock();
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
}