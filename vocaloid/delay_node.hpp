#pragma once
#include "stdafx.h"
#include "maths.hpp"
#include "audio_channel.hpp"
#include "audio_context.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		class DelayNode : public AudioNode {
		private:
			float max_delay_time_;
			int64_t max_delay_size_;
			vector<float> *buffers_;
			int64_t write_index_;
		public:
			AudioParam *delay_time_;

			explicit DelayNode(AudioContext *ctx, float delay_time = 0, float max_delay_time = 1.0f) :AudioNode(ctx), max_delay_time_(max_delay_time) {
				delay_time_ = new AudioParam(ctx);
				delay_time_->value_ = delay_time;
				max_delay_size_ = 0;
				write_index_ = 0;
				buffers_ = new vector<float>[8];
				context_->Connect(delay_time_, this);
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				max_delay_size_ = int64_t((max_delay_time_ + 1) * sample_rate);
				delay_time_->Initialize(sample_rate, frame_size);
				for (auto i = 0; i < channels_; i++) {
					buffers_[i].resize(max_delay_size_);
				}
			}

			int64_t ProcessFrame() override {
				auto delay_buffer = delay_time_->GetResult()->Channel(0)->Data();
				for (auto i = 0; i < frame_size_; i++) {
					auto delay_time = delay_buffer[i];
					delay_time = Clamp(0.0f, max_delay_time_, delay_time);
					auto delay_frames = delay_time * sample_rate_;
					auto read_position = write_index_ + max_delay_size_ - delay_frames;
					if (read_position >= max_delay_size_) {
						read_position -= max_delay_size_;
					}
					auto read_index1 = static_cast<int>(read_position);
					auto read_index2 = (read_index1 + 1) % max_delay_size_;
					float factor = read_position - read_index1;
					for (auto j = 0; j < channels_; j++) {
						auto input = summing_buffer_->Channel(j)->Data()[i];
						buffers_[j][write_index_] = input;
						auto sample1 = buffers_[j][read_index1];
						auto sample2 = buffers_[j][read_index2];
						result_buffer_->Channel(j)->Data()[i] = (1.0f - factor) * sample1 + factor * sample2;
					}
					write_index_ = (write_index_ + 1) % max_delay_size_;
				}
				return frame_size_;
			}
		};
	}
}