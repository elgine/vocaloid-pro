#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "effect.hpp"
#include "effects.h"
#include "load_kernel.hpp"
namespace effect {

	class Cave : public Effect {
	private:
		ConvolutionNode *convolution_;
		AudioChannel* kernel_;
	public:
		explicit Cave(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::CAVE;
			kernel_ = new AudioChannel();
			convolution_ = new ConvolutionNode(ctx);
			LoadKernel(IDR_LARGE_LONG_ECHO_HALL, L"wav", kernel_);
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