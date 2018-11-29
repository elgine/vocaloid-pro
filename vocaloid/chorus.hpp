#include "effect.hpp"
#include "wave_shaper_node.hpp"
#include "biquad_node.hpp"
#include "delay_node.hpp"
#include "oscillator_node.hpp"
#include "gain_node.hpp"

namespace vocaloid {
	namespace effect {
		using namespace vocaloid::node;

		class Chorus : public Effect {
		public:
			struct ChorusOptions {
				float chorus_delay;
				float chorus_speed;
				float chorus_depth;
			};

			DelayNode *delay_;
			GainNode *gain_;
			OscillatorNode *osc_;

			Chorus(AudioContext *ctx) :Effect(ctx) {
				delay_ = new DelayNode(ctx);
				osc_ = new OscillatorNode(ctx);
				gain_ = new GainNode(ctx);

				ctx->Connect(osc_, gain_);
				ctx->Connect(gain_, delay_->delay_time_);
				ctx->Connect(input_, wet_);
				ctx->Connect(input_, delay_);
				ctx->Connect(delay_, wet_);

				osc_->Start();
			}

			void SetOptions(ChorusOptions options) {
				delay_->delay_time_->value_ = options.chorus_delay;
				gain_->gain_->value_ = options.chorus_depth;
				osc_->SetFrequency(options.chorus_speed);
			}
		};
	}
}