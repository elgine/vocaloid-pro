#pragma once
#include "stdafx.h"
#include "maths.hpp"
#include "window.hpp"
namespace vocaloid {
	namespace dsp {

		// Fast Fourier Transformation
		class FFT {
		protected:
			int64_t* reverse_table_;
			float* sin_table_;
			float* cos_table_;
			float* rev_real_;
			float* rev_imag_;
			int64_t buffer_size_;

		public:
			float* real_;
			float* imag_;

			static float CalculateMagnitude(float real, float imag) {
				return sqrtf(powf(real, 2.0f) + powf(imag, 2.0f));
			}

			static float CalculatePhase(float real, float imag) {
				return atan2f(imag, real);
			}

			static float MapRadianToPi(float rad) {
				double a = rad + M_PI,
					b = -2 * M_PI;
				return float(a - (int)(a / b) * b + M_PI);
			}

			FFT() {
				real_ = nullptr;
				imag_ = nullptr;
				reverse_table_ = nullptr;
				rev_real_ = nullptr;
				rev_imag_ = nullptr;
				sin_table_ = nullptr;
				cos_table_ = nullptr;
				buffer_size_ = 1024;
			}

			void Cepstrum(float *cepstrum) {
				for (auto i = 0; i < buffer_size_; i++) {
					cepstrum[i] = log(CalculateMagnitude(real_[i], imag_[i]));
				}
				auto imag = new float[buffer_size_] {0};
				Inverse(cepstrum, imag, buffer_size_, cepstrum);
				delete[] imag;
				imag = nullptr;
			}

			void Initialize(int64_t buffer_size) {
				buffer_size_ = buffer_size;
				DeleteArray(&real_);
				AllocArray(buffer_size_, &real_);
				DeleteArray(&imag_);
				AllocArray(buffer_size_, &imag_);
				DeleteArray(&rev_real_);
				AllocArray(buffer_size_, &rev_real_);
				DeleteArray(&rev_imag_);
				AllocArray(buffer_size_, &rev_imag_);
				DeleteArray(&reverse_table_);
				AllocArray(buffer_size_, &reverse_table_);
				int64_t limit = 1;
				auto bit = buffer_size_ >> 1;
				int i;
				while (limit < buffer_size_) {
					for (i = 0; i < limit; i++) {
						reverse_table_[i + limit] = Clamp((int64_t)0, buffer_size_ - 1, reverse_table_[i] + bit);
						if (reverse_table_[i + limit] >= buffer_size_) {
							throw "Exception of reverse table";
						}
					}
					limit = limit << 1;
					bit = bit >> 1;
				}
				DeleteArray(&sin_table_);
				AllocArray(buffer_size_, &sin_table_);
				DeleteArray(&cos_table_);
				AllocArray(buffer_size_, &cos_table_);
				for (i = 0; i < buffer_size_; i++) {
					sin_table_[i] = (float)sin(-M_PI / i);
					cos_table_[i] = (float)cos(-M_PI / i);
				}
			}

			int64_t GetBufferSize() {
				return buffer_size_;
			}

			void Forward(const float *buffer, int64_t buffer_len) {
				float k = floorf(logf(buffer_size_) / 0.693f);
				if (pow(2, k) != buffer_size_) {
					throw "Invalid buffer size, must be a power of 2.";
				}
				if (buffer_size_ != buffer_len) {
					throw "Supplied buffer is not the same size as defined FFT. ";
				}

				float phase_shift_step_real, phase_shift_step_imag,
					cur_phase_shift_real, cur_phase_shift_imag,
					tr, ti, tmp_real;
				int i, half_size = 1, off;
				for (i = 0; i < buffer_size_; i++) {
					if (reverse_table_[i] >= buffer_size_)throw "Reverse table index too big";
					real_[i] = buffer[reverse_table_[i]];
					imag_[i] = 0.0f;
				}

				while (half_size < buffer_size_) {
					phase_shift_step_real = cos_table_[half_size];
					phase_shift_step_imag = sin_table_[half_size];

					cur_phase_shift_real = 1.0f;
					cur_phase_shift_imag = 0.0f;

					for (int fft_step = 0; fft_step < half_size; fft_step++) {
						i = fft_step;
						while (i < buffer_size_) {
							off = i + half_size;
							tr = (cur_phase_shift_real * real_[off]) - (cur_phase_shift_imag * imag_[off]);
							ti = (cur_phase_shift_real * imag_[off]) + (cur_phase_shift_imag * real_[off]);

							real_[off] = real_[i] - tr;
							imag_[off] = imag_[i] - ti;
							real_[i] += tr;
							imag_[i] += ti;

							i += half_size << 1;
						}

						tmp_real = cur_phase_shift_real;
						cur_phase_shift_real = (tmp_real * phase_shift_step_real) - (cur_phase_shift_imag * phase_shift_step_imag);
						cur_phase_shift_imag = (tmp_real * phase_shift_step_imag) + (cur_phase_shift_imag * phase_shift_step_real);
					}
					half_size = half_size << 1;
				}
			}

			void Inverse(float *output) {
				Inverse(real_, imag_, buffer_size_, output);
				for (int i = 0; i < buffer_size_; i++) {
					output[i] /= buffer_size_;
				}
			}

			// Do IFFT
			void Inverse(const float* real, const float* imag, int64_t len, float *output) {
				int half_size = 1, off, i;
				float phase_shift_step_real, phase_shift_step_imag,
					cur_phase_shift_real, cur_phase_shift_imag,
					tr, ti, tmp_real;
				/*for (i = 0; i < buffer_size_; i++) {
					imag[i] *= -1.0f;
				}*/

				for (i = 0; i < buffer_size_; i++) {
					rev_real_[i] = real[reverse_table_[i]];
					rev_imag_[i] = imag[reverse_table_[i]] * -1.0f;
				}
				auto real_c = rev_real_;
				auto imag_c = rev_imag_;

				while (half_size < buffer_size_) {
					phase_shift_step_real = cos_table_[half_size];
					phase_shift_step_imag = sin_table_[half_size];
					cur_phase_shift_real = 1.0f;
					cur_phase_shift_imag = 0.0f;

					for (int fft_step = 0; fft_step < half_size; fft_step++) {
						i = fft_step;
						while (i < buffer_size_) {
							off = i + half_size;
							tr = (cur_phase_shift_real * real_c[off]) - (cur_phase_shift_imag * imag_c[off]);
							ti = (cur_phase_shift_real * imag_c[off]) + (cur_phase_shift_imag * real_c[off]);
							real_c[off] = real_c[i] - tr;
							imag_c[off] = imag_c[i] - ti;
							real_c[i] += tr;
							imag_c[i] += ti;
							i += half_size << 1;
						}
						tmp_real = cur_phase_shift_real;
						cur_phase_shift_real = (tmp_real * phase_shift_step_real) - (cur_phase_shift_imag * phase_shift_step_imag);
						cur_phase_shift_imag = (tmp_real * phase_shift_step_imag) + (cur_phase_shift_imag * phase_shift_step_real);
					}
					half_size = half_size << 1;
				}

				for (i = 0; i < buffer_size_; i++) {
					output[i] = real_c[i];
				}
			}

			void Dispose() {
				DeleteArray(&real_);
				DeleteArray(&imag_);
				DeleteArray(&reverse_table_);
				DeleteArray(&rev_imag_);
				DeleteArray(&rev_real_);
				DeleteArray(&sin_table_);
				DeleteArray(&cos_table_);
			}
		};
	}
}