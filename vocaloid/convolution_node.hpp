#pragma once
#include "audio_context.hpp"
#include "convolution.hpp"
using namespace vocaloid::dsp;
namespace vocaloid {
	namespace node {
		class ConvolutionNode : public AudioNode {
		protected:
			Convolution **convolver_;

			float CalculateNormalizationScale(AudioChannel* buf) {
				auto gain_calibration = 0.00125;
				auto gain_calibration_sample_rate = 44100;
				auto min_power = 0.000125;
				// Normalize by RMS power.
				auto channels = buf->Channels();
				auto length = buf->Size();
				auto power = 0.0f;
				for (auto i = 0; i < channels; i++) {
					auto channel_power = 0;
					auto channel_data = buf->Channel(i)->Data();
					for (auto j = 0; j < length; j++) {
						auto sample = channel_data[j];
						channel_power += sample * sample;
					}
					power += channel_power;
				}
				power = sqrt(power / (channels * length));
				// Protect against accidental overload.
				if (!isfinite(power) || isnan(power) || power < min_power)
					power = min_power;
				auto scale = 1 / power;
				// Calibrate to make perceived volume same as unprocessed.
				scale *= gain_calibration;
				// Scale depends on sample-rate.
				if (buf->sample_rate_)
					scale *= gain_calibration_sample_rate / buf->sample_rate_;
				// True-stereo compensation.
				if (channels == 4)
					scale *= 0.5;
				return scale;
			}
		public:

			AudioChannel *kernel_;

			explicit ConvolutionNode(AudioContext *ctx) :AudioNode(ctx) {
				convolver_ = new Convolution*[8]{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
				kernel_ = nullptr;
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				for (auto i = 0; i < channels_; i++) {
					if (convolver_[i] == nullptr)convolver_[i] = new Convolution();
					convolver_[i]->Initialize(frame_size, channels_, kernel_->Channel(i)->Data(), kernel_->Size());
				}
			}

			int64_t ProcessFrame() override {
				if (kernel_->Size() > 0) {
					for (auto i = 0; i < channels_;i++) {
						convolver_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
					}
				}	
				return frame_size_;
			}

			int64_t SuggestFrameSize() override {
				return NextPow2(kernel_->Size());
			}
		};
	}
}