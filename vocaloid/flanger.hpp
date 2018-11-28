#include "effect.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
namespace vocaloid {
	namespace effect {
		using namespace vocaloid::node;

		class Flanger : public Effect {
		public:
			struct FlangerOptions {
				float flanger_delay;
				float flanger_depth;
				float flanger_feedback;
				float flanger_speed;
			};

			OscillatorNode *osc_;
			DelayNode *delay_;
			GainNode *gain_;
			GainNode *feedback_;

			Flanger(AudioContext *ctx):Effect(ctx) {
				osc_ = new OscillatorNode(ctx);
				delay_ = new DelayNode(ctx);
				gain_ = new GainNode(ctx);
				feedback_ = new GainNode(ctx);

				ctx->Connect(osc_, gain_);
				ctx->Connect(gain_, delay_->delay_time_);
				ctx->Connect(input_, wet_);
				ctx->Connect(input_, delay_);
				ctx->Connect(delay_, wet_);
				ctx->Connect(delay_, feedback_);
				ctx->Connect(feedback_, input_);

				osc_->Start();
			}

			void SetOptions(FlangerOptions options) {
				osc_->SetFrequency(options.flanger_speed);
				feedback_->gain_->value_ = options.flanger_feedback;
				gain_->gain_->value_ = options.flanger_depth;
				delay_->delay_time_->value_ = options.flanger_delay;
			}
		};
	}
}