#pragma once
#include "audio_context.hpp"
#include "convolution.hpp"
namespace vocaloid {
	namespace node {
		class ConvolutionNode : public AudioNode {
		private:
			dsp::Convolution **convolutions_;
		public:
			vector<float> kernel_;

			explicit ConvolutionNode(AudioContext *ctx) :AudioNode(ctx) {
				convolutions_ = new dsp::Convolution*[8]{ nullptr };
			}

			void Initialize(uint32_t sample_rate, uint64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				for (auto i = 0; i < channels_; i++) {
					if(convolutions_[i] == nullptr || !convolutions_[i])
						convolutions_[i] = new dsp::Convolution(frame_size);
					convolutions_[i]->Initialize(kernel_, kernel_.size());
				}
			}

			int64_t ProcessFrame() override {
				for (auto i = 0; i < channels_; i++) {
					convolutions_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
				}
				return frame_size_;
			}
		};
	}
}