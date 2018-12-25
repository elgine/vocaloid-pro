#include "effect.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "effects.h"
namespace effect {
	struct AlienOptions: EffectOptions{
		float delay;
		float frequency;
	};

	class Alien : public Effect {
	private:
		DelayNode *delay_;
		OscillatorNode *osc_;
		GainNode *osc_gain_;
	public:
		explicit Alien(AudioContext *ctx) :Effect(ctx) {
			delay_ = new DelayNode(ctx);
			osc_ = new OscillatorNode(ctx);
			osc_gain_ = new GainNode(ctx);
			ctx->Connect(osc_, osc_gain_);
			ctx->Connect(osc_gain_, delay_->delay_time_);
			ctx->Connect(delay_, gain_);
		}

		void Start() override {
			osc_->Start();
		}

		AudioNode* Input() override {
			return delay_;
		}

		void SetOptions(AlienOptions options) {
			Effect::SetOptions(options);
			delay_->delay_time_->value_ = options.delay;
			osc_->SetFrequency(options.frequency);
		}
	};
}