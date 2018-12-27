#include "composite.hpp"
#include "oscillator_node.hpp"
#include "delay_node.hpp"
#include "gain_node.hpp"

namespace vocaloid {
	namespace composite {
		using namespace vocaloid::node;
		class Vibrato : public Composite {
		public:
			struct VibratoOptions {
				// 3.5, [0.5, 15]
				float vibrato_delay;
				// 0.03 [0.005, 0.055]
				float vibrato_depth;
				// 0.002, [0.0005, 0.004]
				float vibrato_speed;
			};

			OscillatorNode *osc_;
			DelayNode *delay_;
			GainNode *gain_;

			Vibrato(AudioContext *ctx) : Composite(ctx) {
				delay_ = new DelayNode(ctx);
				osc_ = new OscillatorNode(ctx);
				gain_ = new GainNode(ctx);
				ctx->Connect(osc_, gain_);
				ctx->Connect(gain_, delay_->delay_time_);
				ctx->Connect(input_, delay_);
				ctx->Connect(delay_, wet_);

				SetOptions({
					3.5f,
					0.03f,
					0.002f
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
				Composite::Dispose();
			}

			void SetOptions(VibratoOptions options) {
				delay_->delay_time_->value_ = options.vibrato_delay;
				gain_->gain_->value_ = options.vibrato_depth;
				osc_->SetFrequency(options.vibrato_speed);
			}
		};
	}
}