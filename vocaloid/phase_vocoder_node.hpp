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

			int64_t ProcessFrame(bool flush = false) override {
				int64_t processed = 0;
				for (int i = 0; i < channels_; i++) {
					shifters_[i]->SetTempo(tempo_);
					shifters_[i]->SetPitch(pitch_);
					processed = shifters_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
				}
				return processed;
			}

			/*int64_t Process(bool flush = false) override {
				PullBuffers();
				if (!enable_ || !channels_)return 0;
				return ProcessFrame(flush);;
			}

			int64_t ProcessFrame(bool flush = false) override {
				int64_t processed = 0;
				if (!flush || (flush && !has_flush_)) {
					for (int i = 0; i < channels_; i++) {
						shifters_[i]->SetTempo(tempo_);
						shifters_[i]->SetPitch(pitch_);
						processed = shifters_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
					}
					if (flush) {
						for (int i = 0; i < channels_; i++) {
							shifters_[i]->SetTempo(tempo_);
							shifters_[i]->SetPitch(pitch_);
							shifters_[i]->Process(summing_buffer_->Channel(i)->Data(), shifters_[i]->HopSizeAna());
						}
						has_flush_ = true;
					}
				}
				else {
					for (int i = 0; i < channels_; i++) {
						shifters_[i]->SetTempo(tempo_);
						shifters_[i]->SetPitch(pitch_);
						processed = shifters_[i]->Flush(result_buffer_->Channel(i)->Data(), frame_size_);
					}
					if (processed <= 0)processed = EOF;
				}
				if (processed > 0)
					result_buffer_->silence_ = false;
				else
					result_buffer_->silence_ = true;
				return processed;
			}*/
		};
	}
}
