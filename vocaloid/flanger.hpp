#include "composite.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
namespace vocaloid {
	namespace composite {
		using namespace vocaloid::node;

		class Flanger : public Composite {
		public:
			struct FlangerOptions {
				// 0.25, [0.05, 5]
				float flanger_speed;
				// 0.005, [0.001, 0.02]
				float flanger_delay;
				// 0.002, [0.0005, 0.005]
				float flanger_depth;
				// 0.5, [0, 1]
				float flanger_feedback;
			};

			OscillatorNode *osc_;
			DelayNode *delay_;
			GainNode *gain_;
			GainNode *feedback_;

			Flanger(AudioContext *ctx):Composite(ctx) {
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

				SetOptions({
					0.25f,
					0.005f,
					0.002f,
					0.5f
				});
			}

			void Start() override {
				osc_->Start();
			}

			void Dispose() override {
				osc_->Dispose();
				delete osc_;
				osc_ = nullptr;

				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;

				gain_->Dispose();
				delete gain_;
				gain_ = nullptr;

				feedback_->Dispose();
				delete feedback_;
				feedback_ = nullptr;
				Composite::Dispose();
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