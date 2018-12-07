#pragma once
#include "stdafx.h"
#include "maths.hpp"
#include "audio_channel.hpp"
#include "audio_context.hpp"
#include "convolution_node.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		class ReverbNode : public ConvolutionNode {
		private:
			void BuiltSample(int32_t sample_rate) {
				kernel_len_ = int64_t(sample_rate * time_);
				float n;
				for (auto i = 0; i < kernel_len_; i++) {
					n = reverse_ ? kernel_len_ - i : i;
					kernel_[i] = (float(rand())/RAND_MAX * 2 - 1) * powf(1 - n / kernel_len_, decay_);
				}
			}

		public:
			float decay_;
			float time_;
			bool reverse_;

			ReverbNode(AudioContext *ctx):ConvolutionNode(ctx){
				decay_ = 0.01;
				time_ = 0.01;
				reverse_ = false;
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				BuiltSample(sample_rate);
				ConvolutionNode::Initialize(sample_rate, frame_size);
			}
		};
	}
}