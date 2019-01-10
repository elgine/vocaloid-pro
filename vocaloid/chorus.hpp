#pragma once
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

			static float CHORUS_SPEED_DEFAULT;
			static float CHORUS_SPEED_MIN;
			static float CHORUS_SPEED_MAX;

			static float CHORUS_DELAY_DEFAULT;
			static float CHORUS_DELAY_MIN;
			static float CHORUS_DELAY_MAX;

			static float CHORUS_DEPTH_DEFAULT;
			static float CHORUS_DEPTH_MIN;
			static float CHORUS_DEPTH_MAX;

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

			explicit Chorus(BaseAudioContext *ctx) :Composite(ctx) {
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

			~Chorus() {
				delete delay_;
				delay_ = nullptr;
				delete gain_;
				gain_ = nullptr;
				delete osc_;
				osc_ = nullptr;
			}

			void Start() override {
				osc_->Start();
			}

			void SetOptions(ChorusOptions options) {
				delay_->delay_time_->value_ = options.chorus_delay;
				gain_->gain_->value_ = options.chorus_depth;
				osc_->SetFrequency(options.chorus_speed);
			}
		};

		float Chorus::CHORUS_SPEED_DEFAULT = 3.5f;
		float Chorus::CHORUS_SPEED_MIN = 0.5f;
		float Chorus::CHORUS_SPEED_MAX = 15.0f;
		float Chorus::CHORUS_DELAY_DEFAULT = 0.03f;
		float Chorus::CHORUS_DELAY_MIN = 0.005f;
		float Chorus::CHORUS_DELAY_MAX = 0.055f;
		float Chorus::CHORUS_DEPTH_DEFAULT = 0.002f;
		float Chorus::CHORUS_DEPTH_MIN = 0.0005f;
		float Chorus::CHORUS_DEPTH_MAX = 0.004f;
	}
}