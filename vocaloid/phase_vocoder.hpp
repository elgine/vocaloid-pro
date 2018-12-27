#pragma once
#include "../utility/buffer.hpp"
#include "fft.hpp"
#include "maths.hpp"
#include "window.hpp"
#include "overlap_add.hpp"
#include <iostream>
#include <algorithm>
namespace vocaloid {
	namespace dsp {

		class PhaseVocoder: public OverlapAdd{
		private:
			int64_t sample_rate_;
			float* omega_;
			float* real_;
			float* imag_;
			float *last_phase_;
			float *new_phase_;

			float overlap_;

			float stretch_;
			float pitch_;
			float tempo_;

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
				hop_syn_ = (int64_t)floor(hop_ana_ * stretch_);
				stretch_ = float(hop_syn_) / hop_ana_;
			}

		public:

			explicit PhaseVocoder():stretch_(1.0f),
									pitch_(1.0f),
									tempo_(1.0f), OverlapAdd(){
				input_queue_ = new Buffer<float>();
				output_queue_ = new Buffer<float>();
				out_ = new Buffer<float>();
				frame_ = nullptr;
				win_ = nullptr;
				buffer_ = nullptr;
				last_phase_ = nullptr;
				new_phase_ = nullptr;
				omega_ = nullptr;
				real_ = nullptr;
				imag_ = nullptr;
			}

			void Initialize(int64_t fft_size, float overlap, WINDOW_TYPE win = WINDOW_TYPE::HAMMING, int64_t sample_rate = 44100) {
				OverlapAdd::Initialize(fft_size, fft_size * overlap, fft_size * overlap, win);
				sample_rate_ = sample_rate;
				overlap_ = overlap;

				DeleteArray(&real_);
				AllocArray(fft_size, &real_);
				DeleteArray(&imag_);
				AllocArray(fft_size, &imag_);

				DeleteArray(&last_phase_);
				AllocArray(fft_size, &last_phase_);

				DeleteArray(&new_phase_);
				AllocArray(fft_size, &new_phase_);

				DeleteArray(&omega_);
				AllocArray(fft_size, &omega_);
				for (int i = 0; i < fft_size; i++) {
					omega_[i] = (float)(M_PI * 2.0f * hop_ana_ * i / fft_size);
				}
				input_queue_->Alloc(fft_size * 2);
				input_queue_->SetSize(fft_size);
				output_queue_->Alloc(fft_size * 2);
			}

			void Analyse() override {
				ShiftWindow(frame_, frame_size_);
				memcpy(real_, frame_, sizeof(float) * frame_size_);
				memset(imag_, 0, sizeof(float) * frame_size_);
				FFT(real_, imag_, frame_size_, 1);
			}

			void Processing() override {
				for (int i = 0; i < frame_size_; i++) {
					float magn = CalculateMagnitude(real_[i], imag_[i]);
					float phase = CalculatePhase(real_[i], imag_[i]);
					float diff = WrapToPi(phase - last_phase_[i] - omega_[i]);
					float freq_diff = omega_[i] + diff;
					float new_phase = WrapToPi(new_phase_[i] + freq_diff * stretch_);
					new_phase_[i] = new_phase;
					last_phase_[i] = phase;
					real_[i] = cosf(new_phase) * magn;
					imag_[i] = sinf(new_phase) * magn;
				}
			}

			void Synthesis() override {
				FFT(real_, imag_, frame_size_, -1);
				memcpy(frame_, real_, sizeof(float) * frame_size_);
				ShiftWindow(frame_, frame_size_);
			}

			int64_t Process(float *in, int64_t len, float *out) {
				OverlapAdd::Process(in, len);
				return PopFrame(out, len);
			}

			int64_t PopFrame(float *frame, int64_t len) {
				auto frame_len = int64_t(len * stretch_);
				out_->Alloc(frame_len);
				out_->SetSize(frame_len);
				frame_len = OverlapAdd::Pop(out_, frame_len);
				if (frame_len <= 0)return 0;
				return Resample(out_->Data(), frame_len, len, frame);
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
				Dispose();
				DeleteArray(&win_);
				DeleteArray(&buffer_);
				DeleteArray(&out_);
				DeleteArray(&omega_);
				DeleteArray(&frame_);
				DeleteArray(&last_phase_);
				DeleteArray(&new_phase_);
			}
		};
	}
}