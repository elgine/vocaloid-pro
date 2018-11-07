#pragma once
#include <math.h>
#include "source_node.hpp"
#include "waveform.hpp"
#include "../utility/buffer.hpp"
#include "maths.hpp"
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class OscillatorNode : public SourceNode {
		private:
			float frequency_;
			dsp::WAVEFORM_TYPE type_;
			Buffer<float> *waveform_buffer_;
			bool waveform_dirty_;
			uint64_t offset_;

		public:
			explicit OscillatorNode(AudioContext *ctx) :SourceNode(ctx) {
				frequency_ = 440;
				waveform_dirty_ = true;
				type_ = dsp::WAVEFORM_TYPE::SINE;
				waveform_buffer_ = new Buffer<float>();
				offset_ = 0;
			}

			int64_t Process(Frame *in) override {
				// Resample
				if (waveform_dirty_) {
					uint32_t sample_rate = context_->GetSampleRate();
					auto size = uint64_t(sample_rate / frequency_);
					waveform_buffer_->Alloc(size);
					waveform_buffer_->SetSize(size);
					GenWaveform(type_, size, waveform_buffer_->Data());
					waveform_dirty_ = false;
				}
				in->Alloc(channels_, frame_size_);
				in->SetSize(frame_size_);
				// Fill buffer
				uint64_t size = 0;
				uint64_t fill_size = 0;
				uint64_t buffer_size = waveform_buffer_->Size();
				while (size < frame_size_) {
					fill_size = min(frame_size_ - size, buffer_size - offset_);
					for (auto i = 0; i < channels_; i++)
						in->Data()[i]->Set(waveform_buffer_->Data(), fill_size, offset_, size);
					size += fill_size;
					offset_ = (offset_ + fill_size) % buffer_size;
				}
				return frame_size_;
			}

			void SetFrequency(float frequency) {
				frequency_ = frequency;
				waveform_dirty_ = true;
			}

			void SetWaveformType(dsp::WAVEFORM_TYPE wave_type) {
				type_ = wave_type;
				waveform_dirty_ = true;
			}

			void SetWaveform(float frequency, dsp::WAVEFORM_TYPE wave_type) {
				frequency_ = frequency;
				type_ = wave_type;
				waveform_dirty_ = true;
			}

			dsp::WAVEFORM_TYPE WaveformType() {
				return type_;
			}

			float Frequency() {
				return frequency_;
			}
		};
	}
}