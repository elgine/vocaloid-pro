#pragma once
#include "node.hpp"
#include "audio_param.hpp"
#include "audio_context.hpp"
using namespace std;
namespace vocaloid {

	namespace node {
		class GainNode : public Node {
		public:
			AudioParam *gain_;

			explicit GainNode(AudioContext *ctx, float v = 1.0f) :Node(ctx) {
				gain_ = new AudioParam();
				gain_->value_ = v;
			}

			void Initialize(uint64_t frame_size) override {
				Node::Initialize(frame_size);
				gain_->Initialize(context_->GetSampleRate(), frame_size);
			}

			int64_t Process(Frame *in) override {
				gain_->ComputingValues();
				for (auto i = 0; i < frame_size_; i++) {
					auto value = gain_->ResultBuffer()->Data()[i];
					if (value < 0)value = 0;
					for (auto j = 0; j < channels_; j++) {
						in->Channel(j)->Data()[i] = value * summing_buffer_->Channel(j)->Data()[i];
					}
				}
				return frame_size_;
			}
		};
	}
}