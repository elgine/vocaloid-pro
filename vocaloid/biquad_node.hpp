#pragma once
#include "audio_context.hpp"
#include "biquad.hpp"
namespace vocaloid {
	namespace node {
		class BiquadNode : public AudioNode {
		private:
			dsp::Biquad **filters_;
		public:
			AudioParam *frequency_;
			AudioParam *detune_;
			AudioParam *Q_;
			AudioParam *gain_;
			dsp::BIQUAD_TYPE type_;

			explicit BiquadNode(AudioContext *ctx) :AudioNode(ctx) {
				type_ = dsp::BIQUAD_TYPE::LOW_PASS;
				filters_ = new dsp::Biquad*[8]{nullptr};
				frequency_ = new AudioParam();
				detune_ = new AudioParam();
				Q_ = new AudioParam();
				gain_ = new AudioParam();
				frequency_->value_ = 350;
				detune_->value_ = 0;
				Q_->value_ = 1;
				gain_->value_ = 0;
				RegisterAudioParam(frequency_);
				RegisterAudioParam(detune_);
				RegisterAudioParam(Q_);
				RegisterAudioParam(gain_);
			}

			void Initialize(int32_t sample_rate, int64_t frame_size) override {
				AudioNode::Initialize(sample_rate, frame_size);
				for (auto i = 0; i < channels_; i++) {
					if(filters_[i] == nullptr || !filters_[i])
						filters_[i] = new dsp::Biquad(sample_rate_);
					filters_[i]->SetType(type_);
					filters_[i]->SetParams(frequency_->value_, Q_->value_, gain_->value_, detune_->value_);
				}
				frequency_->Initialize(sample_rate, frame_size);
				detune_->Initialize(sample_rate, frame_size);
				Q_->Initialize(sample_rate, frame_size);
				gain_->Initialize(sample_rate, frame_size);
			}

			int64_t ProcessFrame() override {
				auto frequency_buffer = frequency_->Result()->Channel(0)->Data();
				auto gain_buffer = gain_->Result()->Channel(0)->Data();
				auto Q_buffer = Q_->Result()->Channel(0)->Data();
				auto detune_buffer = detune_->Result()->Channel(0)->Data();
				for (auto i = 0; i < channels_; i++) {
					filters_[i]->Process(summing_buffer_->Channel(i)->Data(),
										frequency_buffer,
										gain_buffer,
										Q_buffer,
										detune_buffer,
										frame_size_,
					                    result_buffer_->Channel(i)->Data());
				}
				return frame_size_;
			}
		};
	}
}