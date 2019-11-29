#pragma once
#include "composite.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/wave_shaper_node.hpp"
namespace vocaloid {
	namespace composite {

		class Tremolo : public Composite {
		private:
			GainNode* tremolo_gain_;
			OscillatorNode* lfo_;
			WaveShaperNode* shaper_;

			float* shaper_buf_;
		public:

			struct TremoloOptions {
				float speed;
				float depth;
			};

			static float TREMOLO_SPEED_DEFAULT;
			static float TREMOLO_SPEED_MIN;
			static float TREMOLO_SPEED_MAX;

			static float TREMOLO_DEPTH_DEFAULT;
			static float TREMOLO_DEPTH_MIN;
			static float TREMOLO_DEPTH_MAX;

			explicit Tremolo(BaseAudioContext *ctx) :Composite(ctx) {
				lfo_ = new OscillatorNode(ctx);
				tremolo_gain_ = new GainNode(ctx);
				tremolo_gain_->gain_->value_ = 0.0;
				shaper_ = new WaveShaperNode(ctx);
				AllocArray(2, &shaper_buf_);
				shaper_buf_[0] = 0.0;
				shaper_buf_[1] = 1.0;
				shaper_->SetCurve(shaper_buf_, 2);

				ctx->Connect(lfo_, shaper_);
				ctx->Connect(shaper_, tremolo_gain_->gain_);
				ctx->Connect(input_, tremolo_gain_);
				ctx->Connect(tremolo_gain_, wet_);
			}

			void SetOptions(TremoloOptions opts) {
				lfo_->SetFrequency(Clamp(TREMOLO_SPEED_MIN, TREMOLO_SPEED_MAX, opts.speed));
				float* new_shaper_buf = nullptr;
				AllocArray(2, &new_shaper_buf);
				new_shaper_buf[0] = 1 - opts.depth;
				new_shaper_buf[1] = 1;
				shaper_->SetCurve(new_shaper_buf, 2);
				DeleteArray(&shaper_buf_);
				shaper_buf_ = new_shaper_buf;
			}

			void Start() override {
				lfo_->Start();
			}

			void Dispose() override {
				if (lfo_) {
					lfo_->Dispose();
					delete lfo_;
					lfo_ = nullptr;
				}
				if (tremolo_gain_) {
					tremolo_gain_->Dispose();
					delete tremolo_gain_;
					tremolo_gain_ = nullptr;
				}
				if (shaper_) {
					shaper_->Dispose();
					delete shaper_;
					shaper_ = nullptr;
				}
				Composite::Dispose();
			}
		};

		float Tremolo::TREMOLO_SPEED_DEFAULT = 4;
		float Tremolo::TREMOLO_SPEED_MIN = 0;
		float Tremolo::TREMOLO_SPEED_MAX = 20;

		float Tremolo::TREMOLO_DEPTH_DEFAULT = 1;
		float Tremolo::TREMOLO_DEPTH_MIN = 0;
		float Tremolo::TREMOLO_DEPTH_MAX = 1;
	}
}