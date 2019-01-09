#pragma once
#include "phase_vocoder.hpp"
namespace vocaloid {
	namespace node {
		class PhaseVocoderNode : public AudioNode {
		private:
			dsp::PhaseVocoder **shifters_;
			float overlap_;
		public:
			float pitch_;
			float tempo_;

			explicit PhaseVocoderNode(BaseAudioContext *ctx) :AudioNode(ctx) {
				shifters_ = new dsp::PhaseVocoder*[8]{ nullptr };
				pitch_ = 1.0f;
				tempo_ = 1.0f;
				overlap_ = 0.25f;
				channels_ = 1;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) {
				AudioNode::Initialize(sample_rate, frame_size);
				for (int i = 0; i < channels_; i++) {
					if (shifters_[i] == nullptr) {
						shifters_[i] = new dsp::PhaseVocoder();
					}
					shifters_[i]->Initialize(frame_size, overlap_);
				}
				return SUCCEED;
			}

			int64_t ProcessFrame() override {
				int64_t processed = 0;
				for (int i = 0; i < channels_; i++) {
					shifters_[i]->SetTempo(tempo_);
					shifters_[i]->SetPitch(pitch_);
					processed = shifters_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
				}
				return processed;
			}

			void Dispose() override {
				for (int i = 0; i < channels_; i++) {
					if (shifters_[i]) {
						delete shifters_[i];
						shifters_[i] = nullptr;
					}
				}
				AudioNode::Dispose();
			}
		};
	}
}
