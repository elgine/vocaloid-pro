#pragma once
#include "biquad_node.hpp"
#include "composite.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;
namespace vocaloid {
	namespace composite {
		class Equalizer : public Composite {
		public:

			static int BIQUAD_COUNT;
			static float* FREQUENCYS;

			BiquadNode **biquads_;

			explicit Equalizer(BaseAudioContext *ctx) :Composite(ctx) {
				biquads_ = new BiquadNode*[BIQUAD_COUNT]{
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx),
					new BiquadNode(ctx)
				};
				for (auto i = 0; i < BIQUAD_COUNT; i++) {
					biquads_[i]->Q_->value_ = 1.4;
					biquads_[i]->gain_->value_ = 0.0f;
					biquads_[i]->frequency_->value_ = FREQUENCYS[i];
					biquads_[i]->type_ = BIQUAD_TYPE::PEAKING;
					if (i == 0) {
						ctx->Connect(input_, biquads_[i]);
					}
					else {
						ctx->Connect(biquads_[i - 1], biquads_[i]);
						if (i == BIQUAD_COUNT - 1) {
							ctx->Connect(biquads_[i], wet_);
						}
					}
				}
			}

			void Dispose() override {
				for (auto i = 0; i < BIQUAD_COUNT; i++) {
					biquads_[i]->Dispose();
					delete biquads_[i];
				}
				delete biquads_;
				biquads_ = nullptr;
				Composite::Dispose();
			}

			void SetOptions(double* options) {
				for (auto i = 0; i < BIQUAD_COUNT; i++) {
					biquads_[i]->gain_->value_ = options[i];
				}
			}
		};

		int Equalizer::BIQUAD_COUNT = 10;
		float* Equalizer::FREQUENCYS = new float[10]{
			30, 60, 125, 250, 500, 1000, 2000, 4000, 8000,
			16000
		};
	}
}