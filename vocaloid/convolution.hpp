#pragma once
#include "stdafx.h"
#include "fft.hpp"
#include "maths.hpp"
// Segmented fft convolution
namespace vocaloid {
	namespace dsp {	
		class Convolution{
		private:
			int64_t kernel_size_;
			int64_t input_size_;
			int64_t fft_size_;
			vector<float> input_;
			vector<float> buffer_;
			FFT *kernel_;
			FFT *main_;
		protected:
			void Processing() {
				for (int i = 0; i < fft_size_; i++) {
					float a = main_->real_[i];
					float b = main_->imag_[i];
					float c = kernel_->real_[i];
					float d = kernel_->imag_[i];
					main_->real_[i] = a * c - b * d;
					main_->imag_[i] = b * c + a * d;
				}
			}
		public:

			explicit Convolution(int64_t input_size) :input_size_(input_size), kernel_size_(0), fft_size_(input_size) {
				kernel_ = new FFT();
				main_ = new FFT();
			}

			void Initialize(vector<float> k, int64_t kernel_len) {
				kernel_size_ = kernel_len;
				fft_size_ = kernel_size_ + input_size_ - 1;
				if ((fft_size_ & (fft_size_ - 1)) != 0) {
					fft_size_ = NextPow2(fft_size_);
				}
				input_.reserve(fft_size_);
				for (auto i = input_.size(); i < fft_size_; i++) {
					input_.emplace_back(0);
				}
				buffer_.reserve(fft_size_);
				for (auto i = buffer_.size(); i < fft_size_; i++) {
					buffer_.emplace_back(0);
				}

				vector<float> kernel(fft_size_, 0);
				for (auto i = 0; i < kernel_len; i++) {
					kernel[i] = k[i];
				}

				kernel_->Dispose();
				kernel_->Initialize(fft_size_);
				kernel_->Forward(kernel, fft_size_);
				main_->Dispose();
				main_->Initialize(fft_size_);
			}

			int64_t Process(vector<float> in, int64_t len, vector<float> &out) {
				// Zero padding right
				for (int i = 0; i < fft_size_; i++) {
					if (i >= len) {
						input_[i] = 0;
					}
					else
						input_[i] = in[i];
				}
				// Forward fft
				main_->Forward(input_, fft_size_);
				// Do processing
				Processing();
				// Do inverse fft
				main_->Inverse(input_);
				// Do overlap add method
				for (int i = 0; i < fft_size_; i++) {
					buffer_[i] += input_[i];
					if (i < input_size_) {
						out[i] = input_[i];
					}
				}
				// Move items left
				for (int i = 0; i < fft_size_; i++) {
					if (i + input_size_ >= fft_size_) {
						buffer_[i] = 0.0f;
					}
					else
						buffer_[i] = buffer_[i + input_size_];
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