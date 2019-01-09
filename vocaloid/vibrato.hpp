#pragma once
#include "composite.hpp"
#include "oscillator_node.hpp"
#include "delay_node.hpp"
#include "gain_node.hpp"

namespace vocaloid {
	namespace composite {
		using namespace vocaloid::node;
		class Vibrato : public Composite {
		private:
			OscillatorNode *osc_;
			DelayNode *delay_;
			GainNode *gain_;
		public:
			static float VIBRATO_DELAY_DEFAULT;
			static float VIBRATO_DELAY_MIN;
			static float VIBRATO_DELAY_MAX;

			static float VIBRATO_DEPTH_DEFAULT;
			static float VIBRATO_DEPTH_MIN;
			static float VIBRATO_DEPTH_MAX;

			static float VIBRATO_SPEED_DEFAULT;
			static float VIBRATO_SPEED_MIN;
			static float VIBRATO_SPEED_MAX;

			struct VibratoOptions {
				// 3.5, [0.5, 15]
				float vibrato_delay;
				// 0.03 [0.005, 0.055]
				float vibrato_depth;
				// 0.002, [0.0005, 0.004]
				float vibrato_speed;
			};

		    explicit Vibrato(BaseAudioContext *ctx) : Composite(ctx) {
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

			void SetDelay(float v) {
				delay_->delay_time_->value_ = Clamp(VIBRATO_DELAY_MIN, VIBRATO_DELAY_MAX, v);
			}

			void SetDepth(float v) {
				gain_->gain_->value_ = Clamp(VIBRATO_DEPTH_MIN, VIBRATO_DELAY_MAX, v);
			}

			void SetSpeed(float v) {
				osc_->SetFrequency(Clamp(VIBRATO_SPEED_MIN, VIBRATO_SPEED_MAX, v));
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

		float Vibrato::VIBRATO_DELAY_DEFAULT = 3.5f;
		float Vibrato::VIBRATO_DELAY_MIN = 0.5f;
		float Vibrato::VIBRATO_DELAY_MAX = 15.0f;
		float Vibrato::VIBRATO_DEPTH_DEFAULT = 0.03f;
		float Vibrato::VIBRATO_DEPTH_MIN = 0.005f;
		float Vibrato::VIBRATO_DEPTH_MAX = 0.055f;
		float Vibrato::VIBRATO_SPEED_DEFAULT = 0.002f;
		float Vibrato::VIBRATO_SPEED_MIN = 0.0005f;
		float Vibrato::VIBRATO_SPEED_MAX = 0.004f;
	}
}