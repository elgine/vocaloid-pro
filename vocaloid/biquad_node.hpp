#pragma once
#include "node.hpp"
#include "audio_param.hpp"
#include "audio_context.hpp"
#include "biquad.hpp"
namespace vocaloid {
	namespace node {
		class BiquadNode : public Node {
		private:
			dsp::Biquad **filters_;
		public:
			AudioParam *frequency_;
			AudioParam *detune_;
			AudioParam *Q_;
			AudioParam *gain_;
			dsp::BIQUAD_TYPE type_;

			explicit BiquadNode(AudioContext *ctx) :Node(ctx) {
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
			}

			void Initialize(uint64_t frame_size) override {
				Node::Initialize(frame_size);
				auto sample_rate = context_->GetSampleRate();
				for (auto i = 0; i < channels_; i++) {
					if(filters_[i] == nullptr || !filters_[i])
						filters_[i] = new dsp::Biquad(sample_rate);
					filters_[i]->SetType(type_);
					filters_[i]->SetParams(frequency_->value_, Q_->value_, gain_->value_, detune_->value_);
				}
				frequency_->Initialize(sample_rate, frame_size);
				detune_->Initialize(sample_rate, frame_size);
				Q_->Initialize(sample_rate, frame_size);
				gain_->Initialize(sample_rate, frame_size);
			}

			int64_t Process(Frame *in) override {
				frequency_->ComputingValues();
				detune_->ComputingValues();
				Q_->ComputingValues();
				gain_->ComputingValues();
				for (auto i = 0; i < channels_; i++) {
					filters_[i]->Process(summing_buffer_->Channel(i)->Data(),
					                        frequency_->ResultBuffer()->Data(),
					                        gain_->ResultBuffer()->Data(),
					                        Q_->ResultBuffer()->Data(),
					                        detune_->ResultBuffer()->Data(),
					                        frame_size_,
					                        in->Channel(i)->Data());
				}
				return frame_size_;
			}
		};
	}
}