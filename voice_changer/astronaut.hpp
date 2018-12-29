#pragma once
#include "effect.hpp"
#include "effects.h"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/distortion.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"

namespace effect {
	
	class Astronaut : public Effect {
	private:
		BiquadNode *b1_;
		BiquadNode *b2_;
		BiquadNode *b3_;
		BiquadNode *b4_;
		BiquadNode *b5_;
		DynamicsCompressorNode *compressor_;
		DistortionNode *distortion_;
	public:
		explicit Astronaut(AudioContext *ctx) :Effect(ctx) {
			id_ = Effects::ASTRONAUT;
			distortion_ = new DistortionNode(ctx);
			b1_ = new BiquadNode(ctx);
			b1_->frequency_->value_ = 1300;
			b2_ = new BiquadNode(ctx);
			b2_->frequency_->value_ = 2000;
			b3_ = new BiquadNode(ctx);
			b3_->frequency_->value_ = 2000;
			b4_ = new BiquadNode(ctx);
			b4_->frequency_->value_ = 500;
			b4_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			b5_ = new BiquadNode(ctx);
			b5_->frequency_->value_ = 500;
			b5_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			compressor_ = new DynamicsCompressorNode(ctx);

			ctx->Connect(b1_, distortion_);
			ctx->Connect(distortion_, b2_);
			ctx->Connect(b2_, b3_);
			ctx->Connect(b3_, b4_);
			ctx->Connect(b4_, b5_);
			ctx->Connect(b5_, compressor_);
			ctx->Connect(compressor_, gain_);
		}

		void SetLowpass1Frequency(float v) {
			b1_->frequency_->value_ = v;
		}

		void SetLowpass2Frequency(float v) {
			b2_->frequency_->value_ = v;
		}

		void SetLowpass3Frequency(float v) {
			b3_->frequency_->value_ = v;
		}

		void SetHighpass1Frequency(float v) {
			b4_->frequency_->value_ = v;
		}

		void SetHighpass2Frequency(float v) {
			b5_->frequency_->value_ = v;
		}

		void Dispose() override {
			b1_->Dispose();
			b2_->Dispose();
			b3_->Dispose();
			b4_->Dispose();
			b5_->Dispose();

			delete b1_;
			delete b2_;
			delete b3_;
			delete b4_;
			delete b5_;

			b1_ = nullptr;
			b2_ = nullptr;
			b3_ = nullptr;
			b4_ = nullptr;
			b5_ = nullptr;

			Effect::Dispose();
		}

		AudioNode *Input() {
			return b1_;
		}
	};
}