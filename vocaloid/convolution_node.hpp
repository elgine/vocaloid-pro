#pragma once
#include "audio_context.hpp"
#include "convolution.hpp"
namespace vocaloid {
	namespace node {
		class ConvolutionNode : public AudioNode {
		private:
			dsp::Convolution **convolutions_;
			float *kernel_;
			uint64_t kernel_len_;
		public:

			explicit ConvolutionNode(AudioContext *ctx) :AudioNode(ctx) {
				convolutions_ = new dsp::Convolution*[8]{ nullptr };
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				for (auto i = 0; i < channels_; i++) {
					if (convolutions_[i] == nullptr) {
						convolutions_[i] = new dsp::Convolution();
					}
					convolutions_[i]->Initialize(frame_size, kernel_, kernel_len_);
				}
			}

			void SetKernel(float *k, uint64_t kernel_len) {
				kernel_ = k;
				kernel_len_ = kernel_len;
			}

			int64_t ProcessFrame() override {
				for (auto i = 0; i < channels_; i++) {
					convolutions_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
				}
				return frame_size_;
			}

			uint64_t SuggestFrameSize() override {
				return kernel_len_;
			}
		};
	}
}