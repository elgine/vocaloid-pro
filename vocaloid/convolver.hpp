#pragma once
#include "stdafx.h"
#include "fft.hpp"
#include "maths.hpp"
#include "../utility/buffer.hpp"
#include <mutex>
// Segmented fft convolution
namespace vocaloid {
	namespace dsp {
		class Convolver {
		private:
			int64_t kernel_size_;
			int64_t input_size_;
			int64_t fft_size_;
			Buffer<float> *output_;
			float *buffer_;
			float *kernel_real_;
			float *kernel_imag_;
			float *main_real_;
			float *main_imag_;

			float normal_scale_;

			thread *thread_;
			mutex process_mutex_;
		public:

			explicit Convolver() :kernel_size_(0), input_size_(0), fft_size_(0) {
				kernel_real_ = nullptr;
				kernel_imag_ = nullptr;
				main_real_ = nullptr;
				main_imag_ = nullptr;
				buffer_ = nullptr;
				output_ = new Buffer<float>();
				normal_scale_ = 1.0f;
			}

			void Initialize(int64_t input_size, uint16_t channels, float* k, int64_t kernel_len, float normal_scale = 1.0f) {
				input_size_ = input_size;
				kernel_size_ = kernel_len;
				normal_scale_ = normal_scale;
				fft_size_ = kernel_size_ + input_size_ - 1;
				if ((fft_size_ & (fft_size_ - 1)) != 0) {
					fft_size_ = NextPow2(fft_size_);
				}

				output_->Alloc(fft_size_);

				DeleteArray(&buffer_);
				AllocArray(fft_size_, &buffer_);

				DeleteArray(&kernel_real_);
				AllocArray(fft_size_, &kernel_real_);

				memcpy(kernel_real_, k, kernel_len * sizeof(float));

				DeleteArray(&kernel_imag_);
				AllocArray(fft_size_, &kernel_imag_);

				FFT(kernel_real_, kernel_imag_, fft_size_, 1);

				DeleteArray(&main_real_);
				AllocArray(fft_size_, &main_real_);

				DeleteArray(&main_imag_);
				AllocArray(fft_size_, &main_imag_);
			}

			int64_t Process(float *in, int64_t len) {
				// Forward fft
				memcpy(main_real_, in, sizeof(float) * len);
				memset(main_imag_, 0, sizeof(float) * fft_size_);
				FFT(main_real_, main_imag_, fft_size_, 1);
				// Do processing
				for (int i = 0; i < fft_size_; i++) {
					float a = main_real_[i];
					float b = main_imag_[i];
					float c = kernel_real_[i];
					float d = kernel_imag_[i];
					main_real_[i] = a * c - b * d;
					main_imag_[i] = b * c + a * d;
				}
				// Do inverse fft
				FFT(main_real_, main_imag_, fft_size_, -1);
				// Do overlap add method
				for (int i = 0; i < fft_size_; i++) {
					buffer_[i] += main_real_[i];
				}
				output_->Add(buffer_, input_size_);
				// Move items left
				for (int i = 0; i < fft_size_; i++) {
					if (i + input_size_ >= fft_size_) {
						buffer_[i] = 0.0f;
					}
					else
						buffer_[i] = buffer_[i + input_size_];
				}
			}

			int64_t PopFrame(float *out, int64_t len) {
				return output_->Pop(out, len);
			}
		};
	}
}
