#pragma once
#include "stdafx.h"
#include "window.hpp"
#include "../utility/buffer.hpp"

namespace vocaloid {
	namespace dsp {
		class OverlapAdd {
		protected:
			int64_t hop_ana_;
			int64_t hop_syn_;
			Buffer<float>* input_queue_;
			Buffer<float>* output_queue_;
			Buffer<float> *out_;
			int64_t frame_size_;
			float *buffer_;
			float *frame_;
			float *win_;

			virtual void PushToOutputQueue(float *b, int64_t len) {
				output_queue_->Add(b, len);
			}

		public:

			explicit OverlapAdd():hop_ana_(0), hop_syn_(0){
				input_queue_ = new Buffer<float>();
				output_queue_ = new Buffer<float>();
				out_ = new Buffer<float>();
				buffer_ = nullptr;
				frame_ = nullptr;
				win_ = nullptr;
			}

			void Initialize(int64_t frame_size, int64_t hop_ana, int64_t hop_syn, WINDOW_TYPE win_type) {
				frame_size_ = frame_size;
				hop_ana_ = hop_ana;
				hop_syn_ = hop_syn;

				DeleteArray(&buffer_);
				AllocArray(frame_size_, &buffer_);
				DeleteArray(&frame_);
				AllocArray(frame_size_, &frame_);
				DeleteArray(&win_);
				AllocArray(frame_size_, &win_);
				GenerateWin(win_type, frame_size, win_);

				input_queue_->Alloc(frame_size * 2);
				input_queue_->SetSize(hop_ana_);
				output_queue_->Alloc(frame_size * 2);
			}

			// Analyse step
			virtual void Analyse() = 0;

			// Processing step
			virtual void Processing() = 0;

			// Sythesis step
			// Apply change to frame array
			virtual void Synthesis() = 0;

			virtual void Process(float *in, int64_t len) {
				// Add to input data queue
				input_queue_->Add(in, len);
				Process();
			}

			virtual void Process() {
				int64_t offset = 0;
				auto data = input_queue_->Data();
				auto input_buffer_size = input_queue_->Size();

				while (input_buffer_size >= offset + frame_size_) {
					for (int i = 0; i < frame_size_; i++) {
						frame_[i] = data[offset + i] * win_[i];
					}
					Analyse();
					Processing();
					Synthesis();
					// Overlap add
					for (int i = 0; i < frame_size_; i++) {
						buffer_[i] += frame_[i] * win_[i];
					}
					PushToOutputQueue(buffer_, hop_syn_);
					for (int i = 0; i < frame_size_; i++) {
						if (i + hop_syn_ >= frame_size_) {
							buffer_[i] = 0.0f;
						}
						else
							buffer_[i] = buffer_[i + hop_syn_];
					}
					offset += hop_ana_;
				}
				input_queue_->Splice(offset);
			}

			virtual int64_t Pop(Buffer<float> *output, int64_t len, bool flush = false) {
				return Pop(output->Data(), len, flush);
			}

			virtual int64_t Pop(float *output, int64_t len, bool flush = false) {
				int64_t out_buffer_len = output_queue_->Size();
				if (out_buffer_len < len && !flush)return 0;
				auto pop_size = min(out_buffer_len, len);
				output_queue_->Pop(output, pop_size);
				return pop_size;
			}

			virtual void Clear() {
				input_queue_->Clear();
				output_queue_->Clear();
			}

			int64_t InputLeft() {
				return input_queue_->Size();
			}

			int64_t HopSizeAna() {
				return hop_ana_;
			}

			int64_t HopSizeSyn() {
				return hop_syn_;
			}

			int64_t OutputLeft() {
				return output_queue_->Size();
			}

			virtual void Dispose() {
				if (input_queue_) {
					delete input_queue_;
					input_queue_ = nullptr;
				}
				if (output_queue_) {
					delete output_queue_;
					output_queue_ = nullptr;
				}
				
				if (out_) {
					delete out_;
					out_ = nullptr;
				}
				DeleteArray(&buffer_);
				DeleteArray(&frame_);
				DeleteArray(&win_);
			}
		};
	}
}