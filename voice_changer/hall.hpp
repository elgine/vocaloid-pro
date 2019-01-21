#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "effect.hpp"
#include "effects.h"
#include "load_kernel.hpp"
namespace effect {

	class Hall : public Effect {
	private:
		ConvolutionNode *convolution_;
		AudioChannel *kernel_;
	public:
		explicit Hall(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::HALL;
			convolution_ = new ConvolutionNode(ctx);
			kernel_ = new AudioChannel();
			LoadKernel(IDR_CHURCH, L"wav", kernel_);
			convolution_->kernel_ = kernel_;
			ctx->Connect(convolution_, gain_);
		}

		void Dispose() override {
			if (convolution_) {
				convolution_->Dispose();
				delete convolution_;
				convolution_ = nullptr;
			}
			if (kernel_) {
				kernel_->Dispose();
				delete kernel_;
				kernel_ = nullptr;
			}
			Effect::Dispose();
		}

		AudioNode *Input() {
			return convolution_;
		}
	};
}