#pragma once
#include "audio_context.hpp"
#include "biquad_node.hpp"
#include "composite.hpp"
namespace vocaloid {
	namespace composite {
		using namespace vocaloid;
		using namespace vocaloid::node;
		using namespace vocaloid::dsp;

		class Equalizer3Band : public Composite {
		public:
			struct Equalizer3BandOptions {
				float low_frequency;
				float low_gain;
				float middle_frequency;
				float middle_gain;
				float high_frequency;
				float high_gain;

				Equalizer3BandOptions(float lo_freq, float lo_gain, float mi_freq, float mi_gain, float hi_freq, float hi_gain) {
					low_frequency = lo_freq;
					low_gain = lo_gain;
					middle_frequency = mi_freq;
					middle_gain = mi_gain;
					high_frequency = hi_freq;
					high_gain = hi_gain;
				}
			};

			BiquadNode *hi_;
			BiquadNode *lo_;
			BiquadNode *mi_;

			explicit Equalizer3Band(AudioContext *ctx):Composite(ctx){
				hi_ = new BiquadNode(ctx);
				hi_->type_ = BIQUAD_TYPE::HIGH_SHELF;
				hi_->frequency_->value_ = 2500;
				hi_->gain_->value_ = -10;
				lo_ = new BiquadNode(ctx);
				lo_->type_ = BIQUAD_TYPE::LOW_SHELF;
				lo_->frequency_->value_ = 100;
				lo_->gain_->value_ = 1;
				mi_ = new BiquadNode(ctx);
				mi_->type_ = BIQUAD_TYPE::PEAKING;
				mi_->frequency_->value_ = 900;
				mi_->gain_->value_ = -7;
				ctx->Connect(input_, hi_);
				ctx->Connect(hi_, mi_);
				ctx->Connect(mi_, lo_);
				ctx->Connect(lo_, wet_);
			}

			void SetOptions(Equalizer3BandOptions options) {
				lo_->frequency_->value_ = options.low_frequency;
				lo_->gain_->value_ = options.low_gain;
				mi_->frequency_->value_ = options.middle_frequency;
				mi_->gain_->value_ = options.middle_gain;
				hi_->frequency_->value_ = options.high_frequency;
				hi_->gain_->value_ = options.high_gain;
			}

			void Dispose() override {
				hi_->Dispose();
				delete hi_;
				hi_ = nullptr;

				lo_->Dispose();
				delete lo_;
				lo_ = nullptr;

				mi_->Dispose();
				delete mi_;
				mi_ = nullptr;
				Composite::Dispose();
			}
		};
	}
}