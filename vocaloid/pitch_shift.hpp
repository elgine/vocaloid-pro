#pragma once
#include "fft.hpp"
#include "maths.hpp"
#include "window.hpp"
#include <iostream>
namespace vocaloid {
	namespace dsp {
		class PitchShift {
		private:
			FFT *fft_;
			float* win_;
			float* omega_;
			Buffer<float>* input_queue_;
			Buffer<float>* output_queue_;
			float *out_;
			float *buffer_;
			float *prev_in_phase_;
			float *prev_out_phase_;
			float *frame_;
			int64_t overlap_size_;
			int64_t hop_size_;
			int64_t hop_size_s_;
			float stretch_;
			float pitch_;
			float tempo_;
			

			void Processing() {
				for (int i = 0; i < fft_->GetBufferSize(); i++) {
					float magn = FFT::CalculateMagnitude(fft_->real_[i], fft_->imag_[i]);
					float phase = FFT::CalculatePhase(fft_->real_[i], fft_->imag_[i]);
					float diff = phase - prev_in_phase_[i] - omega_[i];
					float freq_diff = omega_[i] + FFT::MapRadianToPi(diff);
					float new_phase = FFT::MapRadianToPi(prev_out_phase_[i] + freq_diff * stretch_);
					prev_out_phase_[i] = new_phase;
					prev_in_phase_[i] = phase;
					fft_->real_[i] = cosf(new_phase) * magn;
					fft_->imag_[i] = sinf(new_phase) * magn;
				}
			}

			void ShiftWindow(float *data, int64_t len) {
				int64_t halfLen = len / 2;
				if (len % 2 == 0) {
					for (int i = 0; i < halfLen; i++) {
						float tmp = data[i];
						data[i] = data[i + halfLen];
						data[i + halfLen] = tmp;
					}
				}
				else {
					int64_t shiftAmt = halfLen,
						remaining = len,
						curr = 0;
					float save = data[curr];
					while (remaining >= 0) {
						float next = data[(curr + shiftAmt) % len];
						data[(curr + shiftAmt) % len] = save;
						save = next;
						curr = (curr + shiftAmt) % len;
						remaining--;
					}
				}
			}

			void UpdateHopSize() {
				stretch_ = pitch_ * tempo_;
				hop_size_s_ = (int64_t)floor(hop_size_ * stretch_);
				stretch_ = float(hop_size_s_) / hop_size_;
			}

		public:

			explicit PitchShift() :hop_size_(0),
				hop_size_s_(0),
				overlap_size_(0),
				stretch_(1.0f),
				pitch_(1.0f),
				tempo_(1.0f) {
				fft_ = new FFT();
				input_queue_ = new Buffer<float>();
				output_queue_ = new Buffer<float>();
				out_ = nullptr;
				frame_ = nullptr;
				win_ = nullptr;
				buffer_ = nullptr;
				prev_in_phase_ = nullptr;
				prev_out_phase_ = nullptr;
				omega_ = nullptr;
			}

			void Initialize(int64_t fft_size, float overlap, WINDOW_TYPE win = WINDOW_TYPE::HAMMING, float extra = 1.0f) {
				fft_->Initialize(fft_size);
				DeleteArray(&out_);
				AllocArray(fft_size, &out_);

				DeleteArray(&frame_);
				AllocArray(fft_size, &frame_);

				DeleteArray(&win_);
				AllocArray(fft_size, &win_);
				GenerateWin(win, fft_size, win_, extra);

				DeleteArray(&buffer_);
				AllocArray(fft_size, &buffer_);

				DeleteArray(&prev_in_phase_);
				AllocArray(fft_size, &prev_in_phase_);

				DeleteArray(&prev_out_phase_);
				AllocArray(fft_size, &prev_out_phase_);

				overlap_size_ = (int64_t)(fft_size * overlap);
				hop_size_s_ = hop_size_ = fft_size - overlap_size_;

				DeleteArray(&omega_);
				AllocArray(fft_size, &omega_);
				for (int i = 0; i < fft_size; i++) {
					omega_[i] = (float)(M_PI * 2.0f * hop_size_ * i / fft_size);
				}
				input_queue_->Alloc(fft_size * 2);
				input_queue_->SetSize(fft_size);
				output_queue_->Alloc(fft_size * 2);
			}

			int64_t Process(float *in, int64_t len, float *out) {
				// Add to input data queue
				input_queue_->Add(in, len);
				int64_t fft_size = fft_->GetBufferSize();
				int64_t offset = 0;
				auto data = input_queue_->Data();
				auto input_buffer_size = input_queue_->Size();
				while (input_buffer_size > offset + fft_size) {
					// Windowing
					for (int i = 0; i < fft_size; i++) {
						frame_[i] = data[offset + i] * win_[i];
					}
					ShiftWindow(frame_, fft_size);
					// Forward fft
					fft_->Forward(frame_, fft_size);
					// Do processing
					 Processing();
					// Do inverse fft
					fft_->Inverse(frame_);
					ShiftWindow(frame_, fft_size);
					// Overlap add
					for (int i = 0; i < fft_size; i++) {
						buffer_[i] += frame_[i] * win_[i];
					}
					output_queue_->Add(buffer_, hop_size_s_);
					// Move items left
					for (int i = 0; i < fft_size; i++) {
						if (i + hop_size_s_ >= fft_size) {
							buffer_[i] = 0.0f;
						}
						else
							buffer_[i] = buffer_[i + hop_size_s_];
					}
					offset += hop_size_;
				}
				input_queue_->Splice(offset);
				return PopFrame(out, len);
			}

			int64_t PopFrame(float *frame, int64_t len) {
				auto frame_len = min((int64_t)output_queue_->Size(), int64_t(len * stretch_));
				if (frame_len < len)return 0;
				output_queue_->Pop(out_, frame_len);
				return Resample(out_, frame_len, len, frame);
			}

			void SetPitch(float v) {
				if (pitch_ == v)return;
				pitch_ = v;
				UpdateHopSize();
			}

			void SetTempo(float v) {
				if (tempo_ == v)return;
				tempo_ = v;
				UpdateHopSize();
			}

			float GetPitch() {
				return pitch_;
			}

			float GetTempo() {
				return tempo_;
			}

			void Dispose() {
				fft_->Dispose();
				DeleteArray(&win_);
				DeleteArray(&buffer_);
				DeleteArray(&out_);
				DeleteArray(&omega_);
				DeleteArray(&frame_);
				DeleteArray(&prev_in_phase_);
				DeleteArray(&prev_out_phase_);
			}
		};
	}
}