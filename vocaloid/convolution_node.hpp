#pragma once
#include "audio_context.hpp"
#include "convolution.hpp"
namespace vocaloid {
	namespace node {
		class ConvolutionNode : public AudioNode {
		protected:
			dsp::Convolution *convolver_;
			float *kernel_;
			int64_t kernel_len_;
		public:

			explicit ConvolutionNode(AudioContext *ctx) :AudioNode(ctx) {
				convolver_ = new dsp::Convolution();
				kernel_ = nullptr;
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				convolver_->Initialize(frame_size, channels_, kernel_, kernel_len_);
			}

			void SetKernel(float *k, int64_t kernel_len) {
				DeleteArray(&kernel_);
				AllocArray(kernel_len, &kernel_);
				memcpy(kernel_, k, sizeof(float) * kernel_len);
				kernel_len_ = kernel_len;
			}

			int64_t ProcessFrame() override {
				if(kernel_len_ > 0)
					return convolver_->Process(summing_buffer_->Data(), summing_buffer_->Size(), result_buffer_->Data());
				return 0;
			}

			int64_t SuggestFrameSize() override {
				return NextPow2(kernel_len_);
			}
		};
	}
}