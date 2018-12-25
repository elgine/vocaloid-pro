#pragma once
#include "fft.hpp"
#include "../utility/buffer.hpp"
#include <mutex>
#include <thread>
#include <atomic>
// Segmented fft convolution
namespace vocaloid {
	namespace dsp {
		class Convolver2 {
		private:
			float **kernel_real_segs_;
			float **kernel_imag_segs_;
			float *kernel_real_;
			float *kernel_imag_;
			float *main_real_;
			float *main_imag_;
			float *buffer_;
			float *segment_;
			float *segment_buffer_;

			bool processing_;

			atomic<int> kernel_segments_;
			atomic<int> seg_index_;
			atomic<int> compose_index_;

			int64_t kernel_size_;
			int64_t input_size_;
			int64_t fft_size_;
			int64_t seg_fft_size_;
			
			Buffer<float>* input_;
			Buffer<float>* output_;

			atomic<float> normal_scale_;

			vector<thread*> thread_pool_;
			mutex process_mutex_;
			condition_variable compose_condition_;
			condition_variable take_condition_;

			void FrameOverlapAdd(float *in, int index) {
				for (int i = 0; i < seg_fft_size_; i++) {
					segment_buffer_[i] += in[i];
					if (isnan(segment_buffer_[i]) || isinf(segment_buffer_[i])) {
						segment_buffer_[i] = 0.0f;
					}
				}
				memcpy(segment_ + index * input_size_, segment_buffer_, input_size_ * sizeof(float));
				// Move items left
				for (int i = 0; i < seg_fft_size_; i++) {
					if (i + input_size_ >= seg_fft_size_) {
						segment_buffer_[i] = 0.0f;
					}
					else
						segment_buffer_[i] = segment_buffer_[i + input_size_];
				}
			}

			void OverlapAdd() {
				// Do overlap add method
				for (int i = 0; i < fft_size_; i++) {
					buffer_[i] += segment_[i] * normal_scale_;
					if (isnan(buffer_[i]) || isinf(buffer_[i])) {
						//printf("invalidate data\n");
						buffer_[i] = 0.0f;
					}
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

			int ExtractFrames(float *main_real, float *main_imag, float *kernel_real, float *kernel_imag) {
				int index = -1;
				{
					unique_lock<mutex> lck(process_mutex_);
					if (input_->Size() < input_size_ || seg_index_ < 0 || seg_index_ >= kernel_segments_) {
						return -1;
					}
					index = seg_index_++;
					memcpy(kernel_real, kernel_real_segs_[index], input_size_ * sizeof(float));
					memcpy(kernel_imag, kernel_imag_segs_[index], input_size_ * sizeof(float));
					memcpy(main_real, main_real_, input_size_ * sizeof(float));
				}
				return index;
			}

			void ComposeFrame(float *main_real, int index) {
				{
					unique_lock<mutex> lck(process_mutex_); 
					while (compose_index_ < index) {
						compose_condition_.wait(lck);
					}
					FrameOverlapAdd(main_real, index);	
					//printf("%d\n", index);
					if (compose_index_ == kernel_segments_ - 1) {
						OverlapAdd();
						NextFrame();
						take_condition_.notify_all();
					}
					else {
						compose_index_++;
					}
					compose_condition_.notify_all();
				}
			}

			void NextFrame() {
				seg_index_ = 0;
				compose_index_ = 0;
				if (input_->Size() >= input_size_) {
					input_->Pop(main_real_, input_size_);
					memset(segment_buffer_, 0, sizeof(float) * seg_fft_size_);
					//printf("New frame-----------------------\n");
				}
				else {
					seg_index_ = -1;
				}
			}

			void ProcessInThread() {
				int64_t fft_size;
				int16_t index;
				float *main_real = nullptr, *main_imag = nullptr, *kernel_real = nullptr, *kernel_imag = nullptr;
				{
					unique_lock<mutex> lck(process_mutex_);
					fft_size = seg_fft_size_;
					AllocArray(fft_size, &main_real);
					AllocArray(fft_size, &main_imag);
					AllocArray(fft_size, &kernel_real);
					AllocArray(fft_size, &kernel_imag);
				}
				while (true) {
					memset(main_real, 0, fft_size * sizeof(float));
					memset(main_imag, 0, fft_size * sizeof(float));
					index = ExtractFrames(main_real, main_imag, kernel_real, kernel_imag);
					if (index > -1) {
						{
							unique_lock<mutex> lck(process_mutex_);
							FFT(main_real, main_imag, fft_size, 1);
						}
						
						for (int i = 0; i < fft_size; i++) {
							float a = main_real[i];
							float b = main_imag[i];
							float c = kernel_real[i];
							float d = kernel_imag[i];
							main_real[i] = a * c - b * d;
							main_imag[i] = b * c + a * d; 
						}
						{
							unique_lock<mutex> lck(process_mutex_);
							FFT(main_real, main_imag, fft_size, -1);
						}
						ComposeFrame(main_real, index);
					}
					this_thread::sleep_for(chrono::milliseconds(4));
				}
			}

			void ProcessBackground(float *in, int64_t size) {
				{
					unique_lock<mutex> lck(process_mutex_);
					input_->Add(in, size);
					if (seg_index_ < 0)NextFrame();

					if (thread_pool_.size() <= 0) {
						processing_ = true;
						thread_pool_.reserve(thread_count_);
						for (auto i = 0; i < thread_count_; i++) {
							thread_pool_.emplace_back(new thread(&Convolver2::ProcessInThread, this));
						}
					}
				}
			}

			int64_t PopFrame(float *out) {
				int64_t len = 0;
				{
					unique_lock<mutex> lck(process_mutex_);
					while (output_->Size() < input_size_) {
						take_condition_.wait(lck);
					}
					len = output_->Pop(out, input_size_);
				}
				return len;
			}

			int64_t ProcessDirect(float *in, int64_t size, float *out) {
				memcpy(main_real_, in, sizeof(float) * size);
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
					if (i < input_size_) {
						out[i] = buffer_[i];
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

		public:

			int16_t thread_count_;

			explicit Convolver2() {
				input_ = new Buffer<float>();
				output_ = new Buffer<float>();
				normal_scale_ = 1.0f;
				kernel_real_segs_ = nullptr;
				kernel_imag_segs_ = nullptr;
				kernel_real_ = nullptr;
				kernel_imag_ = nullptr;
				buffer_ = nullptr;
				segment_ = nullptr;
				main_real_ = nullptr;
				main_imag_ = nullptr;
				segment_buffer_ = nullptr;
				thread_count_ = 4;
				seg_index_ = 0;
				compose_index_ = 0;
			}

			void Initialize(int64_t input_size, float* k, int64_t kernel_len, float normal_scale = 1.0f) {
				seg_index_ = -1;
				compose_index_ = 0;
				processing_ = false;
				for (auto thread : thread_pool_) {
					if (thread->joinable()) {
						thread->join();
					}
				}
				thread_pool_.clear();
				input_size_ = input_size;
				kernel_size_ = kernel_len;
				normal_scale_ = normal_scale;
				fft_size_ = kernel_size_ + input_size_ - 1;
				if ((fft_size_ & (fft_size_ - 1)) != 0) {
					fft_size_ = NextPow2(fft_size_);
				}
				input_->Alloc(fft_size_);
				output_->Alloc(fft_size_);
				output_->SetSize(input_size_);

				DeleteArray(&buffer_);
				AllocArray(fft_size_, &buffer_);

				DeleteArray(&segment_);
				AllocArray(fft_size_, &segment_);
				// Calculate segments divide kernel into
				if (fft_size_ / input_size_ > 1) {
					seg_fft_size_ = input_size_ * 2;
					DeleteArray(&main_real_);
					AllocArray(input_size_, &main_real_);
					DeleteArray(&segment_buffer_);
					AllocArray(seg_fft_size_, &segment_buffer_);
					if (kernel_real_segs_ != nullptr && kernel_imag_segs_ != nullptr) {
						for (auto i = 0; i < kernel_segments_; i++) {
							DeleteArray(kernel_real_segs_ + i);
							DeleteArray(kernel_imag_segs_ + i);
						}
						delete[] kernel_real_segs_;
						kernel_real_segs_ = nullptr;
						delete[] kernel_imag_segs_;
						kernel_imag_segs_ = nullptr;
					}

					kernel_segments_ = fft_size_ / input_size_;
					kernel_real_segs_ = new float*[kernel_segments_];
					kernel_imag_segs_ = new float*[kernel_segments_];
					
					int64_t k_start = 0;
					int64_t k_len = 0;
					for (auto i = 0; i < kernel_segments_; i++) {
						AllocArray(seg_fft_size_, kernel_real_segs_ + i);
						AllocArray(seg_fft_size_, kernel_imag_segs_ + i);
						k_start = input_size_ * i;
						k_len = min(input_size_, kernel_len - k_start);
						if (k_start < kernel_len) {
							memcpy(kernel_real_segs_[i], k + k_start, k_len * sizeof(float));
							FFT(kernel_real_segs_[i], kernel_imag_segs_[i], seg_fft_size_, 1);
						}
					}
				}
				else {
					DeleteArray(&main_real_);
					AllocArray(fft_size_, &main_real_);
					DeleteArray(&main_imag_);
					AllocArray(fft_size_, &main_imag_);

					DeleteArray(&kernel_real_);
					DeleteArray(&kernel_imag_);
					AllocArray(fft_size_, &kernel_real_);
					AllocArray(fft_size_, &kernel_imag_);
					memcpy(kernel_real_, k, kernel_len * sizeof(float));
					FFT(kernel_real_, kernel_imag_, input_size_, 1);
				}
			}

			int64_t Process(float *in, int64_t size, float *out) {
				if (kernel_segments_ > 1) {
					ProcessBackground(in, size);
					return PopFrame(out);
				}
				else {
					ProcessDirect(in, size, out);
				}
			}
		};
	}
}