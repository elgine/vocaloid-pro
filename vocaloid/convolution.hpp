#pragma once
#include "stdafx.h"
#include "fft.hpp"
#include "maths.hpp"
// Segmented fft convolution
namespace vocaloid {
	namespace dsp {
		class Convolution {
		private:
			int64_t kernel_size_;
			int64_t input_size_;
			int64_t fft_size_;
			Buffer<float>* input_;
			Buffer<float>* output_;
			float *kernel_real_;
			float *kernel_imag_;
			float *main_real_;
			float *main_imag_;

			float normal_scale_;
		public:

			explicit Convolution() :kernel_size_(0), input_size_(0), fft_size_(0) {
				kernel_real_ = nullptr;
				kernel_imag_ = nullptr;
				main_real_ = nullptr;
				main_imag_ = nullptr;
				input_ = new Buffer<float>();
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

				input_->Alloc(fft_size_);
				input_->SetSize(fft_size_);
				output_->Alloc(fft_size_);
				output_->SetSize(fft_size_);

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

			int64_t Process(float *in, int64_t len, float *out) {
				input_->Fill(0);
				input_->Set(in, len);
				auto input = input_->Data();
				auto buffer = output_->Data();
				// Forward fft
				memcpy(main_real_, input, sizeof(float) * fft_size_);
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
					buffer[i] += main_real_[i];
					if (i < input_size_) {
						out[i] = buffer[i] * normal_scale_;
					}
				}
				// Move items left
				for (int i = 0; i < fft_size_; i++) {
					if (i + input_size_ >= fft_size_) {
						buffer[i] = 0.0f;
					}
					else
						buffer[i] = buffer[i + input_size_];
				}
				return input_size_;
			}

			int64_t FFTSize() {
				return fft_size_;
			}

			int64_t KernelSize() {
				return kernel_size_;
			}
		};
	}
}
