#pragma once
#include "audio_node.hpp"
#include "audio_timeline.hpp"
namespace vocaloid {
	namespace node {
		enum AudioParamType {
			K_RATE,
			A_RATE
		};

		class AudioParam : public AudioNode, public AudioTimeline {
		private:
			int64_t offset_;
		public:

			explicit AudioParam(BaseAudioContext* ctx) :AudioNode(ctx, AudioNodeType::PARAM, true, true), AudioTimeline() {
				offset_ = 0;
				channels_ = 1;
			}

			void SetChannels(int16_t channels) final {}

			void SetOffset(int64_t offset) {
				offset_ = offset;
			}

			void Offset(int64_t len) {
				offset_ += len;
			}

			int64_t ProcessFrame() {
				if (TimelineEmpty() && summing_buffer_->silence_) {
					result_buffer_->Fill(value_);
				}
				else {
					for (auto i = 0; i < frame_size_; i++) {
						result_buffer_->Channel(0)->Data()[i] = summing_buffer_->Channel(0)->Data()[i] + GetValueAtTime(CalculatePlayedTime(sample_rate_, offset_ + i));
					}
				}
				offset_ += frame_size_;
				return frame_size_;
			}

			void Clear() override {
				offset_ = 0;
			}

			void Dispose() override {
				AudioNode::Dispose();
				AudioTimeline::Dispose();
			}
		};
	}
}