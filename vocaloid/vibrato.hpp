#include "./effect.hpp"
#include "oscillator_node.hpp"
#include "delay_node.hpp"
#include "gain_node.hpp"

namespace vocaloid {
	namespace effect {
		using namespace vocaloid::node;

		class Vibrato : public Effect {
		public:
			struct VibratoOptions {
				float vibrato_delay;
				float vibrato_depth;
				float vibrato_speed;
			};

			OscillatorNode *osc_;
			DelayNode *delay_;
			GainNode *gain_;

			Vibrato(AudioContext *ctx) : Effect(ctx) {
				delay_ = new DelayNode(ctx);
				osc_ = new OscillatorNode(ctx);
				gain_ = new GainNode(ctx);
				ctx->Connect(osc_, gain_);
				ctx->Connect(gain_, delay_->delay_time_);
				ctx->Connect(input_, delay_);
				ctx->Connect(delay_, wet_);
				osc_->Start();
			}

			void SetOptions(VibratoOptions options) {
				delay_->delay_time_->value_ = options.vibrato_delay;
				gain_->gain_->value_ = options.vibrato_depth;
				osc_->SetFrequency(options.vibrato_speed);
			}
		};
	}
}