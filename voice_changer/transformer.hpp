#pragma once
#include "Jungle.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "effects.h"
#include "role.hpp"
using namespace effect;
namespace role {
	
	class Transformer : public Role {

	private:
		AudioNode *input_;
		Jungle *jungle1_;
		Jungle *jungle2_;
		Jungle *jungle3_;
		Jungle *jungle4_;
		DynamicsCompressorNode *compressor_;
	public:
		explicit Transformer(BaseAudioContext *ctx) : Role(ctx) {
			id_ = Roles::TRANSFORMER;
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

		void Dispose() override {
			if (input_) {
				input_->Dispose();
				delete input_;
				input_ = nullptr;
			}
			if (compressor_) {
				compressor_->Dispose();
				delete compressor_;
				compressor_ = nullptr;
			}
			if (jungle1_) {
				jungle1_->Dispose();
				delete jungle1_;
				jungle1_ = nullptr;
			}
			if (jungle2_) {
				jungle2_->Dispose();
				delete jungle2_;
				jungle2_ = nullptr;
			}
			if (jungle3_) {
				jungle3_->Dispose();
				delete jungle3_;
				jungle3_ = nullptr;
			}
			if (jungle4_) {
				jungle4_->Dispose();
				delete jungle4_;
				jungle4_ = nullptr;
			}
			Role::Dispose();
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

		void Resume() override {
			jungle1_->Resume();
			jungle2_->Resume();
			jungle3_->Resume();
			jungle4_->Resume();
		}
	};
}