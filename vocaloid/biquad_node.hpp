#pragma once
#include "audio_context.hpp"
#include "audio_param.hpp"
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
				frequency_ = new AudioParam(ctx);
				detune_ = new AudioParam(ctx);
				Q_ = new AudioParam(ctx);
				gain_ = new AudioParam(ctx);
				frequency_->value_ = 350;
				detune_->value_ = 0;
				Q_->value_ = 1;
				gain_->value_ = 0;
				context_->Connect(frequency_, this);
				context_->Connect(detune_, this);
				context_->Connect(Q_, this);
				context_->Connect(gain_, this);
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
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
				return SUCCEED;
			}

			void Clear() override {
				frequency_->Clear();
				detune_->Clear();
				Q_->Clear();
				gain_->Clear();
			}

			void Dispose() override {
				if (frequency_) {
					delete frequency_;
					frequency_ = nullptr;
				}
				
				if (detune_) {
					delete detune_;
					detune_ = nullptr;
				}

				if (Q_) {
					delete Q_;
					Q_ = nullptr;
				}

				if (gain_) {
					delete gain_;
					gain_ = nullptr;
				}
				
				AudioNode::Dispose();
			}

			int64_t ProcessFrame() override {
				auto frequency_buffer = frequency_->GetResult()->Channel(0)->Data();
				auto gain_buffer = gain_->GetResult()->Channel(0)->Data();
				auto Q_buffer = Q_->GetResult()->Channel(0)->Data();
				auto detune_buffer = detune_->GetResult()->Channel(0)->Data();
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