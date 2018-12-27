#include "composite.hpp"
#include "wave_shaper_node.hpp"
#include "biquad_node.hpp"
#include "delay_node.hpp"
#include "oscillator_node.hpp"
#include "gain_node.hpp"

namespace vocaloid {
	namespace composite {
		using namespace vocaloid::node;

		class Chorus : public Composite {
		public:
			struct ChorusOptions {
				// 3.5, [0.5, 15]
				float chorus_speed;
				// 0.03, [0.005, 0.055]
				float chorus_delay;
				// 0.002, [0.0005, 0.004]
				float chorus_depth;
			};

			DelayNode *delay_;
			GainNode *gain_;
			OscillatorNode *osc_;

			Chorus(AudioContext *ctx) :Composite(ctx) {
				delay_ = new DelayNode(ctx);
				osc_ = new OscillatorNode(ctx);
				gain_ = new GainNode(ctx);

				ctx->Connect(osc_, gain_);
				ctx->Connect(gain_, delay_->delay_time_);
				ctx->Connect(input_, wet_);
				ctx->Connect(input_, delay_);
				ctx->Connect(delay_, wet_);

				SetOptions({
					3.5,
					0.03,
					0.002
				});
			}

			void Start() override {
				osc_->Start();
			}

			void Dispose() override {
				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;

				gain_->Dispose();
				delete gain_;
				gain_ = nullptr;

				osc_->Dispose();
				delete osc_;
				osc_ = nullptr;

				Composite::Dispose();
			}

			void SetOptions(ChorusOptions options) {
				delay_->delay_time_->value_ = options.chorus_delay;
				gain_->gain_->value_ = options.chorus_depth;
				osc_->SetFrequency(options.chorus_speed);
			}
		};
	}
}