#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "effect.hpp"
#include "load_kernel.hpp"
namespace effect {

	class Radio : public Effect {
	private:
		ConvolutionNode *convolution_;
		AudioChannel *kernel_;
	public:
		explicit Radio(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::RADIO;
			convolution_ = new ConvolutionNode(ctx);
			kernel_ = new AudioChannel();
			LoadKernel(IDR_RADIO, L"wav", kernel_);
			convolution_->kernel_ = kernel_;
			ctx->Connect(input_, convolution_);
			ctx->Connect(convolution_, wet_);
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
	};
}