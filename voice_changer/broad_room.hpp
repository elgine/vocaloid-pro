#pragma once
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../utility/buffer.hpp"
#include "effect.hpp"
#include "effects.h"
#include "extract_resource.hpp"
#include "resource.h"
namespace effect {

	class BroadRoom : public Effect {
	private:
		ConvolutionNode *convolution_;
		AudioChannel* kernel_;
	public:
		explicit BroadRoom(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::BROAD_ROOM;
			convolution_ = new ConvolutionNode(ctx);
			kernel_ = new AudioChannel();
			LoadKernel(IDR_LARGE_WIDE_ECHO_HALL, L"wav", kernel_);
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