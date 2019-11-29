#pragma once
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "effect.hpp"
#include "effects.h"

namespace effect {

	class Telephone : public Effect {
	private:
		BiquadNode *b1_;
		BiquadNode *b2_;
		BiquadNode *b3_;
		BiquadNode *b4_;
		DynamicsCompressorNode *compressor_;
	public:
		explicit Telephone(BaseAudioContext *ctx):Effect(ctx) {
			id_ = Effects::TELEPHONE;
			b1_ = new BiquadNode(ctx);
			b1_->frequency_->value_ = 2000;
			b2_ = new BiquadNode(ctx);
			b2_->frequency_->value_ = 2000;
			b3_ = new BiquadNode(ctx);
			b3_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			b3_->frequency_->value_ = 500;
			b4_ = new BiquadNode(ctx);
			b4_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			b4_->frequency_->value_ = 500;
			compressor_ = new DynamicsCompressorNode(ctx);

			ctx->Connect(input_, b1_);
			ctx->Connect(b1_, b2_);
			ctx->Connect(b2_, b3_);
			ctx->Connect(b3_, b4_);
			ctx->Connect(b4_, compressor_);
			ctx->Connect(compressor_, wet_);
		}

		void Dispose() override {
			if (b1_) {
				b1_->Dispose();
				delete b1_; b1_ = nullptr;
			}
			if (b2_) {
				b2_->Dispose();
				delete b2_; b2_ = nullptr;
			}
			if (b3_) {
				b3_->Dispose();
				delete b3_; b3_ = nullptr;
			}
			if (b4_) {
				b4_->Dispose();
				delete b4_; b4_ = nullptr;
			}
			if (compressor_) {
				compressor_->Dispose();
				delete compressor_; compressor_ = nullptr;
			}
			Effect::Dispose();
		}
	};
}