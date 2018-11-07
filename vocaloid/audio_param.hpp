#pragma once
#include <vector>
#include <algorithm>
#include "maths.hpp"
#include "../utility/buffer.hpp"
#include "frame.hpp"
#include "unit.hpp"
#include "timeline.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		enum AudioParamType {
			K_RATE,
			A_RATE
		};

		class AudioParam : public Unit, public Timeline {
		protected:
			Buffer<float> *result_buffer_;
			uint64_t offset_;
			uint32_t sample_rate_;

			uint64_t CalculatePlayedTime(uint64_t offset) {
				return uint64_t((float)(offset) / float(sample_rate_) * 1000.0f);
			}

		public:
			explicit AudioParam(AudioParamType type = AudioParamType::A_RATE) :Unit(bool(type), false, 1) {
				sample_rate_ = 44100;
				value_ = 1.0f;
				offset_ = 0;
				result_buffer_ = new Buffer<float>(frame_size_);
			}

			void Initialize(uint64_t frame_size) final {}

			void Initialize(uint32_t sample_rate, uint64_t frame_size) {
				Unit::Initialize(frame_size);
				sample_rate_ = sample_rate;
				result_buffer_->Alloc(frame_size_);
			}

			virtual void ComputingValues() {
				PullInputs();
				for (auto i = 0; i < frame_size_; i++) {
					result_buffer_->Data()[i] = summing_buffer_->Channel(0)->Data()[i] + GetValueAtTime(CalculatePlayedTime(offset_ + i));
				}
				offset_ += frame_size_;
			}

			void SetOffset(uint64_t offset) {
				offset_ = offset;
			}

			void Offset(uint64_t len) {
				offset_ += len;
			}

			Buffer<float>* ResultBuffer() {
				return result_buffer_;
			}

			uint64_t GetOffset() {
				return offset_;
			}
		};
	}
}