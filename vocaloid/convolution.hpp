#pragma once
#include "stdafx.h"
#include "fft.hpp"
#include "maths.hpp"
#include <thread>
#include <mutex>
#include <memory>
// Segmented fft convolution
namespace vocaloid {
	namespace dsp {	
		class Convolution{
		private:
			int64_t kernel_size_;
			int64_t input_size_;
			int64_t fft_size_;
			uint16_t channels_;
			Buffer<float>** input_;
			Buffer<float>** output_;
			FFT *kernel_;
			FFT *main_;

		public:

			explicit Convolution():kernel_size_(0), input_size_(0), fft_size_(0){
				kernel_ = new FFT();
				main_ = new FFT();
				input_ = new Buffer<float>*[8]{ nullptr };
				output_ = new Buffer<float>*[8]{ nullptr };
			}

			void Initialize(int64_t input_size, uint16_t channels, float* k, int64_t kernel_len) {
				input_size_ = input_size;
				kernel_size_ = kernel_len;
				fft_size_ = kernel_size_ + input_size_ - 1;
				if ((fft_size_ & (fft_size_ - 1)) != 0) {
					fft_size_ = NextPow2(fft_size_);
				}

				channels_ = channels;
				for (auto i = 0; i < channels; i++) {
					if (input_[i] == nullptr)
						input_[i] = new Buffer<float>();
					input_[i]->SetSize(fft_size_);

					if (output_[i] == nullptr)
						output_[i] = new Buffer<float>();
					output_[i]->SetSize(fft_size_);
				}

				kernel_->Dispose();
				kernel_->Initialize(fft_size_); 
				float *kernel = nullptr;
				AllocArray(fft_size_, &kernel);
				memcpy(kernel, k, kernel_len);
				kernel_->Forward(kernel, fft_size_);
				DeleteArray(&kernel);
				main_->Dispose();
				main_->Initialize(fft_size_);
			}

			int64_t Process(Buffer<float> **in, int64_t len, Buffer<float> **out) {
				for (auto channel = 0; channel < channels_; channel++) {
					input_[channel]->Fill(0);
					input_[channel]->Set(in[channel]->Data(), len);
					auto input = input_[channel]->Data();
					auto buffer = output_[channel]->Data();
					// Forward fft
					main_->Forward(input, fft_size_);
					// Do processing
					for (int i = 0; i < fft_size_; i++) {
						float a = main_->real_[i];
						float b = main_->imag_[i];
						float c = kernel_->real_[i];
						float d = kernel_->imag_[i];
						main_->real_[i] = a * c - b * d;
						main_->imag_[i] = b * c + a * d;
					}
					// Do inverse fft
					main_->Inverse(input);
					// Do overlap add method
					for (int i = 0; i < fft_size_; i++) {
						buffer[i] += input[i];
						if (i < input_size_) {
							out[channel]->Data()[i] = buffer[i];
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