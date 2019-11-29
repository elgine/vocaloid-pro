#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "env.hpp"
#include "load_kernel.hpp"
namespace env {

	class Cave : public Env {
	private:
		ConvolutionNode *convolution_;
		AudioChannel* kernel_;
	public:
		explicit Cave(BaseAudioContext *ctx) :Env(ctx) {
			id_ = Envs::CAVE;
			kernel_ = new AudioChannel();
			convolution_ = new ConvolutionNode(ctx);
			LoadKernel(IDR_LARGE_LONG_ECHO_HALL, L"wav", kernel_);
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