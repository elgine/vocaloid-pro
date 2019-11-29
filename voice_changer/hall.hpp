#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "env.hpp"
#include "load_kernel.hpp"
namespace env {

	class Hall : public Env {
	private:
		ConvolutionNode *convolution_;
		AudioChannel *kernel_;
	public:
		explicit Hall(BaseAudioContext *ctx) :Env(ctx) {
			id_ = Envs::HALL;
			convolution_ = new ConvolutionNode(ctx);
			kernel_ = new AudioChannel();
			LoadKernel(IDR_CHURCH, L"wav", kernel_);
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
			Env::Dispose();
		}
	};
}