#pragma once
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/distortion.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "effect.hpp"
#include "effects.h"
namespace effect {
	
	class Megaphone : public Effect {
	private:
		DynamicsCompressorNode *compressor_;
		DistortionNode *distortion_;
		BiquadNode *lpf1_;
		BiquadNode *lpf2_;
		BiquadNode *hpf1_;
		BiquadNode *hpf2_;
	public:
		explicit Megaphone(AudioContext *ctx) :Effect(ctx) {
			id_ = Effects::MEGAPHONE;
			compressor_ = new DynamicsCompressorNode(ctx);
			distortion_ = new DistortionNode(ctx);

			lpf1_ = new BiquadNode(ctx);
			lpf1_->frequency_->value_ = 2000.0f;

			lpf2_ = new BiquadNode(ctx);
			lpf2_->frequency_->value_ = 2000.0f;

			hpf1_ = new BiquadNode(ctx);
			hpf1_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			hpf1_->frequency_->value_ = 500;

			hpf2_ = new BiquadNode(ctx);
			hpf2_->type_ = vocaloid::dsp::BIQUAD_TYPE::HIGH_PASS;
			hpf2_->frequency_->value_ = 500;

			ctx->Connect(lpf1_, lpf2_);
			ctx->Connect(lpf2_, hpf1_);
			ctx->Connect(hpf1_, hpf2_);
			ctx->Connect(hpf2_, distortion_);
			ctx->Connect(distortion_, compressor_);
			ctx->Connect(compressor_, gain_);
		}

		AudioNode *Input() {
			return lpf1_;
		}

		void Dispose() override {
			compressor_->Dispose();
			delete compressor_; compressor_ = nullptr;
			distortion_->Dispose();
			delete distortion_; distortion_ = nullptr;
			lpf1_->Dispose();
			lpf2_->Dispose();
			hpf1_->Dispose();
			hpf2_->Dispose();
			delete lpf1_;
			delete lpf2_;
			delete hpf1_;
			delete hpf2_;
			lpf1_ = nullptr;
			lpf2_ = nullptr;
			hpf1_ = nullptr;
			hpf2_ = nullptr;
			Effect::Dispose();
		}
	};
}