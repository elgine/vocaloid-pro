#pragma once
#include "node.hpp"
#include "convolution.hpp"
namespace vocaloid {
	namespace node {
		class ConvolutionNode : public Node {
		private:
			dsp::Convolution **convolutions_;
		public:
			vector<float> kernel_;

			explicit ConvolutionNode(AudioContext *ctx) :Node(ctx) {
				convolutions_ = new dsp::Convolution*[8]{ nullptr };
			}

			void Initialize(uint64_t frame_size) override {
				Node::Initialize(frame_size);
				for (auto i = 0; i < channels_; i++) {
					if(convolutions_[i] == nullptr || !convolutions_[i])
						convolutions_[i] = new dsp::Convolution(frame_size);
					convolutions_[i]->Initialize(kernel_, kernel_.size());
				}
			}

			int64_t Process(Frame *in) override {
				for (auto i = 0; i < channels_; i++) {
					convolutions_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), in->Channel(i)->Data());
				}
				return frame_size_;
			}
		};
	}
}