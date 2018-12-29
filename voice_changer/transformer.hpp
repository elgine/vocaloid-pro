#pragma once
#include "../vocaloid/Jungle.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "effects.h"
#include "effect.hpp"
using namespace vocaloid::composite;
namespace effect {
	
	class Transformer : public Effect {

	private:
		AudioNode *input_;
		Jungle *jungle1_;
		Jungle *jungle2_;
		Jungle *jungle3_;
		Jungle *jungle4_;
		DynamicsCompressorNode *compressor_;
	public:
		explicit Transformer(AudioContext *ctx) : Effect(ctx) {
			id_ = Effects::TRANSFORMER;
			jungle1_ = new Jungle(ctx);
			jungle1_->SetPitchOffset(-0.1f);
			jungle2_ = new Jungle(ctx);
			jungle2_->SetPitchOffset(-0.2f);
			jungle3_ = new Jungle(ctx);
			jungle3_->SetPitchOffset(-0.4f);
			jungle4_ = new Jungle(ctx);
			jungle4_->SetPitchOffset(-0.8f);

			input_ = new GainNode(ctx);
			compressor_ = new DynamicsCompressorNode(ctx);
			ctx->Connect(input_, jungle1_->input_);
			ctx->Connect(input_, jungle2_->input_);
			ctx->Connect(input_, jungle3_->input_);
			ctx->Connect(input_, jungle4_->input_);
			ctx->Connect(jungle1_->output_, compressor_);
			ctx->Connect(jungle2_->output_, compressor_);
			ctx->Connect(jungle3_->output_, compressor_);
			ctx->Connect(jungle4_->output_, compressor_);
			ctx->Connect(compressor_, gain_);
		}

		AudioNode *Input() {
			return input_;
		}

		void Start() override {
			jungle1_->Start();
			jungle2_->Start();
			jungle3_->Start();
			jungle4_->Start();
		}

		void Dispose() override {
			input_->Dispose();
			delete input_;
			input_ = nullptr;

			compressor_->Dispose();
			delete compressor_;
			compressor_ = nullptr;

			jungle1_->Dispose();
			jungle2_->Dispose();
			jungle3_->Dispose();
			jungle4_->Dispose();

			delete jungle1_;
			delete jungle2_;
			delete jungle3_;
			delete jungle4_;

			jungle1_ = nullptr;
			jungle2_ = nullptr;
			jungle3_ = nullptr;
			jungle4_ = nullptr;

			Effect::Dispose();
		}
	};
}