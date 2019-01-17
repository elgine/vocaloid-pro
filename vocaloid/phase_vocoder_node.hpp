#pragma once
#include "audio_node.hpp"
#include "phase_vocoder.hpp"
namespace vocaloid {
	namespace node {
		class PhaseVocoderNode : public AudioNode {
		private:
			dsp::PhaseVocoder **shifters_;
			float overlap_;
			bool has_flush_;
			int64_t left = 0;
		public:
			float pitch_;
			float tempo_;

			explicit PhaseVocoderNode(BaseAudioContext *ctx) :AudioNode(ctx) {
				shifters_ = new dsp::PhaseVocoder*[8]{ nullptr };
				pitch_ = 1.0f;
				tempo_ = 1.0f;
				overlap_ = 0.25f;
				channels_ = 1;
				left = -1;
				watched_ = true;
			}

			void Clear() override {
				for (int i = 0; i < channels_; i++) {
					if (shifters_[i]) {
						shifters_[i]->Clear();
					}
				}
				AudioNode::Clear();
			}

			void Dispose() override {
				for (int i = 0; i < channels_; i++) {
					if (shifters_[i]) {
						shifters_[i]->Dispose();
						delete shifters_[i];
						shifters_[i] = nullptr;
					}
				}
				AudioNode::Dispose();
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) {
				AudioNode::Initialize(sample_rate, frame_size);
				for (int i = 0; i < channels_; i++) {
					if (shifters_[i] == nullptr) {
						shifters_[i] = new dsp::PhaseVocoder();
					}
					shifters_[i]->Initialize(frame_size, overlap_);
				}
				has_flush_ = false;
				return SUCCEED;
			}

			int64_t Process(bool flush = false) override {
				PullBuffers();
				if (channels_ < 1 || !enable_ || (summing_buffer_->silence_ && shifters_[0]->InputLeft() <= 0 && shifters_[0]->OutputLeft() <= 0))return 0;
				return ProcessFrame();
			}

			int64_t ProcessFrame(bool flush = false) override {
				int64_t processed = 0;
				if (!summing_buffer_->silence_) {
					for (int i = 0; i < channels_; i++) {
						shifters_[i]->SetTempo(tempo_);
						shifters_[i]->SetPitch(pitch_);
						shifters_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size());
					}
				}
				for (int i = 0; i < channels_; i++) {
					shifters_[i]->SetTempo(tempo_);
					shifters_[i]->SetPitch(pitch_);
					processed = shifters_[i]->Pop(result_buffer_->Channel(i)->Data(), frame_size_, true);
				}
				return processed;
			}
		};
	}
}
