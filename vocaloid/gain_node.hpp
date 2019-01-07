#pragma once
#include "audio_node.hpp"
#include "audio_param.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		class GainNode : public AudioNode {
		public:
			AudioParam *gain_;

			explicit GainNode(BaseAudioContext *ctx, float v = 1.0f):AudioNode(ctx) {
				gain_ = new AudioParam(ctx);
				gain_->value_ = v;
				context_->Connect(gain_, this);
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				gain_->Initialize(sample_rate_, frame_size);
				return SUCCEED;
			}

			int64_t ProcessFrame() override {
				auto gain_result = gain_->GetResult()->Channel(0)->Data();
				for (auto i = 0; i < frame_size_; i++) {
					auto value = gain_result[i];
					if (value < 0)value = 0;
					for (auto j = 0; j < channels_; j++) {
						result_buffer_->Channel(j)->Data()[i] = value * summing_buffer_->Channel(j)->Data()[i];
					}
				}
				return frame_size_;
			}

			void Clear() override {
				gain_->Clear();
			}

			void Dispose() override {
				if (gain_) {
					delete gain_;
					gain_ = nullptr;
				}
				AudioNode::Dispose();
			}
		};
	}
}