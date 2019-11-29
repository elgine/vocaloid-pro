#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../utility/buffer.hpp"
#include "load_kernel.hpp"
#include "env.hpp"
#include "extract_resource.hpp"
#include "resource.h"
using namespace vocaloid;
namespace env {

	class BroadRoom : public Env {
	private:
		ConvolutionNode *convolution_;
		AudioChannel* kernel_;
	public:
		explicit BroadRoom(BaseAudioContext *ctx) :Env(ctx) {
			id_ = Envs::BROAD_ROOM;
			convolution_ = new ConvolutionNode(ctx);
			kernel_ = new AudioChannel();
			LoadKernel(IDR_LARGE_WIDE_ECHO_HALL, L"wav", kernel_);
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