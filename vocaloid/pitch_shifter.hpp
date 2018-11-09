#pragma once
#include "pitch_shift.hpp"
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class PitchShifter : public AudioNode {
		private:
			dsp::PitchShift **shifters_;
			float overlap_;
		public:
			float pitch_;
			float tempo_;

			explicit PitchShifter(AudioContext *ctx) :AudioNode(ctx) {
				shifters_ = new dsp::PitchShift*[8]{nullptr};
				pitch_ = 1.0f;
				tempo_ = 1.0f;
				overlap_ = 0.75f;
			}

			void Initialize(uint32_t sample_rate, uint64_t frame_size) {
				AudioNode::Initialize(sample_rate, frame_size);
				for (int i = 0; i < channels_; i++) {
					if (shifters_[i] != nullptr) {
						shifters_[i] = new dsp::PitchShift();
					}
					shifters_[i]->Initialize(frame_size, overlap_);
				}
			}

			int64_t ProcessFrame() override {
				int64_t processed = 0;
				for (int i = 0; i < channels_; i++) {
					shifters_[i]->SetTempo(tempo_);
					shifters_[i]->SetPitch(overlap_);
					processed = shifters_[i]->Process(summing_buffer_->Channel(i)->Data(), summing_buffer_->Size(), result_buffer_->Channel(i)->Data());
				}
				return processed;
			}
		};
	}
}