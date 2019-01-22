#pragma once
#include "convolver.hpp"
#include "audio_node.hpp"
using namespace vocaloid::dsp;
namespace vocaloid {
	namespace node {
		class ConvolutionNode : public AudioNode {
		protected:
			Convolver **convolver_;

			float CalculateNormalizationScale(AudioChannel* buf) {
				auto gain_calibration = 0.00125;
				auto gain_calibration_sample_rate = sample_rate_;
				auto min_power = 0.000125;
				// Normalize by RMS power.
				auto channels = buf->Channels();
				auto length = buf->Size();
				auto power = 0.0f;
				for (auto i = 0; i < channels; i++) {
					float channel_power = 0;
					auto channel_data = buf->Channel(i)->Data();
					for (auto j = 0; j < length; j++) {
						float sample = channel_data[j];
						channel_power += sample * sample;
					}
					power += channel_power;
				}
				power = sqrt(power / (channels * length));
				// Protect against accidental overload.
				if (!isfinite(power) || isnan(power) || power < min_power)
					power = min_power;
				auto scale = 1.0f / power;
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

			bool normalize_;

			explicit ConvolutionNode(BaseAudioContext *ctx) :AudioNode(ctx) {
				convolver_ = new Convolver*[8]{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
				kernel_ = nullptr;
				normalize_ = true;
			}

			void Dispose() override {
#ifdef _DEBUG
#endif
				for (auto i = 0; i < channels_; i++) {
					if (convolver_[i] != nullptr) {
						convolver_[i]->Dispose();
						delete convolver_[i];
						convolver_[i] = nullptr;
					}
				}
				AudioNode::Dispose();
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				auto scale = normalize_?CalculateNormalizationScale(kernel_):1.0f;
				if (kernel_ != nullptr) {
					auto kernel_channels = kernel_->Channels();
					for (auto i = 0; i < channels_; i++) {
						if (convolver_[i] == nullptr)convolver_[i] = new Convolver();
						convolver_[i]->Initialize(frame_size, kernel_->Channel(i % kernel_channels)->Data(), kernel_->Size(), scale);
					}
				}
				return SUCCEED;
			}

			int64_t ProcessFrame(bool flush = false) override {
				if (kernel_->Size() > 0) {
					for (auto i = 0; i < channels_;i++) {
						convolver_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
					}
				}	
				return frame_size_;
			}
		};
	}
}