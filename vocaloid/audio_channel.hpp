#pragma once
#include <math.h>
#include "channel.h"
#include "maths.hpp"
#include "../utility/buffer.hpp"
namespace vocaloid {
	namespace node {
		class AudioChannel {
			typedef Buffer<float>* PBuffer;
		private:
			int16_t channels_;
			PBuffer *data_;
		public:
			bool silence_;

			int32_t sample_rate_;

			explicit AudioChannel(int16_t channels = 2, int64_t max_size = 1024, int32_t sample_rate = 44100) {
				sample_rate_ = sample_rate;
				channels_ = 0;
				data_ = new PBuffer[8]{nullptr};
				silence_ = true;
				Alloc(channels, max_size);
			}

			void Copy(AudioChannel *b, int64_t size = 0, int64_t offset = 0, int64_t dst = 0) {
				if (size <= 0)size = b->Size();
				if (offset + size > b->Size())size = b->Size() - offset;
				Alloc(b->Channels(), size);
				SetSize(b->Size());
				sample_rate_ = b->sample_rate_;
				for (auto i = 0; i < b->Channels(); i++) {
					memcpy(data_[i]->Data() + dst, b->Channel(i)->Data() + offset, size * sizeof(float));
				}
				silence_ = b->silence_;
			}

			void Zero() {
				Fill(0.0f);
				silence_ = true;
			}

			void Add(AudioChannel *b) {
				for (auto i = 0; i < min(channels_, b->Channels()); i++) {
					data_[i]->Add(b->Channel(i)->Data(), b->Size());
				}
			}

			void Splice(int64_t len, int64_t offset = 0) {
				for (auto i = 0; i < channels_; i++) {
					data_[i]->Splice(len, offset);
				}
			}

			void FromBuffer(Buffer<char> *bytes, int16_t bits, int16_t channels) {
				FromByteArray(bytes->Data(), bytes->Size(), bits, channels);
			}

			template<typename T = const char* | vector<char>>
			void FromByteArray(T byte_array, int64_t byte_length, int16_t bits, int16_t channels) {
				int16_t depth = bits / 8;
				int16_t step = depth * channels;
				int64_t len = byte_length / step;
				Alloc(channels, len);
				SetSize(len);
				float max = powf(2.0f, (float)bits - 1) - 1.0f;
				for (int i = 0; i < byte_length; i += step) {
					for (int j = 0; j < channels; j++) {
						long offset = i + j * depth;
						long value = byte_array[offset] & 0xFF;
						for (int k = 1; k < depth; k++) {
							value |= (long)(byte_array[offset + k] << (k * 8));
						}
						data_[j]->Data()[i / step] = value / max;
					}
				}
			}

			template<typename T = char* | vector<char>&>
			void ToByteArray(int16_t bits, T byte_array, int64_t &byte_length) {
				int16_t depth = bits / 8;
				int16_t step = depth * channels_;
				int64_t buffer_size = Size();
				byte_length = step * buffer_size;
				float max = powf(2.0f, (float)bits - 1) - 1.0f;
				for (int i = 0; i < buffer_size; i++) {
					for (int j = 0; j < channels_; j++) {
						float clipped = Clamp(-0.98f, 0.98f, data_[j]->Data()[i]);
						auto value = (long)(clipped * max);
						for (int k = 0; k < depth; k++) {
							byte_array[i * step + j * depth + k] = (char)((value >> 8 * k) & 0xFF);
						}
					}
				}
			}

			void Fill(float v, int64_t len = 0, int64_t offset = 0) {
				len = len <= 0 ? MaxSize() : len;
				for (auto i = 0; i < channels_; i++) {
					data_[i]->Fill(v, len, offset);
				}
			}

			void Mix(AudioChannel *in) {
				silence_ &= in->silence_;
				int16_t out_channels = channels_, in_channels = in->Channels();
				int64_t size = in->Size();
				Alloc(channels_, size);
				if (out_channels == in_channels) {
					for (auto i = 0; i < channels_; i++) {
						for (auto j = 0; j < size; j++) {
							data_[i]->Data()[j] += in->Data()[i]->Data()[j];
						}
					}
					return;
				}
				// Up mixing
				if (out_channels > in_channels) {
					if (out_channels == CHANNEL_STEREO) {
						for (auto j = 0; j < size; j++) {
							data_[Channel::RIGHT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j];
							data_[Channel::LEFT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j];
						}
					}
					else if (out_channels == CHANNEL_QUAD) {
						if (in_channels == CHANNEL_STEREO) {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j];
								data_[Channel::RIGHT]->Data()[j] += in->Data()[Channel::RIGHT]->Data()[j];
							}
						}
						else {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += data_[Channel::RIGHT]->Data()[j] = in->Data()[Channel::LEFT]->Data()[j];
							}
						}
					}
					else if (out_channels == CHANNEL_5_1) {
						if (in_channels == CHANNEL_QUAD) {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j];
								data_[Channel::RIGHT]->Data()[j] += in->Data()[Channel::RIGHT]->Data()[j];
								data_[Channel::SURROUND_LEFT]->Data()[j] += in->Data()[Channel::SURROUND_LEFT]->Data()[j];
								data_[Channel::SURROUND_RIGHT]->Data()[j] += in->Data()[Channel::SURROUND_RIGHT]->Data()[j];
							}
						}
						else if (in_channels == CHANNEL_STEREO) {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j];
								data_[Channel::RIGHT]->Data()[j] += in->Data()[Channel::RIGHT]->Data()[j];
							}
						}
						else {
							for (auto j = 0; j < size; j++) {
								data_[Channel::CENTER]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j];
							}
						}
					}
				}
				// Down mixing
				else {
					if (in_channels == CHANNEL_STEREO) {
						for (auto j = 0; j < size; j++) {
							data_[Channel::LEFT]->Data()[j] += 0.5f * (in->Data()[Channel::LEFT]->Data()[j] + in->Data()[Channel::RIGHT]->Data()[j]);
						}
					}
					else if (in_channels == CHANNEL_QUAD) {
						if (out_channels == CHANNEL_MONO) {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += 0.25f * (in->Data()[Channel::LEFT]->Data()[j] +
									in->Data()[Channel::RIGHT]->Data()[j] +
									in->Data()[Channel::SURROUND_LEFT]->Data()[j] +
									in->Data()[Channel::SURROUND_RIGHT]->Data()[j]);
							}
						}
						else {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += 0.5f * (in->Data()[Channel::LEFT]->Data()[j] + in->Data()[Channel::SURROUND_LEFT]->Data()[j]);
								data_[Channel::RIGHT]->Data()[j] += 0.5f * (in->Data()[Channel::RIGHT]->Data()[j] + in->Data()[Channel::SURROUND_RIGHT]->Data()[j]);
							}
						}
					}
					else {
						if (out_channels == CHANNEL_MONO) {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] +=
									0.7071f * (in->Data()[Channel::LEFT]->Data()[j] + in->Data()[Channel::LEFT]->Data()[j]) +
									in->Data()[Channel::CENTER]->Data()[j] +
									0.5f * (in->Data()[Channel::SURROUND_LEFT]->Data()[j] + in->Data()[Channel::SURROUND_RIGHT]->Data()[j]);
							}
						}
						else if (out_channels == CHANNEL_STEREO) {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j] + 0.7071f * (in->Data()[Channel::CENTER]->Data()[j] + in->Data()[Channel::SURROUND_LEFT]->Data()[j]);
								data_[Channel::RIGHT]->Data()[j] += in->Data()[Channel::RIGHT]->Data()[j] + 0.7071f * (in->Data()[Channel::CENTER]->Data()[j] + in->Data()[Channel::SURROUND_RIGHT]->Data()[j]);
							}
						}
						else {
							for (auto j = 0; j < size; j++) {
								data_[Channel::LEFT]->Data()[j] += in->Data()[Channel::LEFT]->Data()[j] + 0.7071f * (in->Data()[Channel::CENTER]->Data()[j]);
								data_[Channel::RIGHT]->Data()[j] += in->Data()[Channel::RIGHT]->Data()[j] + 0.7071f * (in->Data()[Channel::CENTER]->Data()[j]);
								data_[Channel::SURROUND_LEFT]->Data()[j] += in->Data()[Channel::SURROUND_LEFT]->Data()[j];
								data_[Channel::SURROUND_RIGHT]->Data()[j] += in->Data()[Channel::SURROUND_RIGHT]->Data()[j];
							}
						}
					}
				}
			}

			void Alloc(int16_t channels, int64_t size) {
				if (channels_ < channels) {
					for (auto i = 0; i < channels; i++) {
						if(data_[i] == nullptr)
							data_[i] = new Buffer<float>(Size());
					}
				}
				for (auto i = 0; i < max(channels_, channels); i++) {
					data_[i]->Alloc(size);
				}
				channels_ = channels;
			}

			void SetSize(int64_t size) {
				for (auto i = 0; i < channels_; i++) {
					data_[i]->SetSize(size);
				}
			}

			void Dispose() {
				for (auto i = 0; i < channels_; i++) {
					data_[i]->Dispose();
				}
			}

			int16_t Channels() {
				return channels_;
			}

			int64_t Size() {
				if (channels_ > 0 && data_ != nullptr && data_[0] != nullptr)
					return data_[0]->Size();
				return 0;
			}

			int64_t MaxSize() {
				if (channels_ > 0 && data_ != nullptr && data_[0] != nullptr)
					return data_[0]->MaxSize();
				return 0;
			}

			PBuffer Channel(int16_t index) {
				return data_[index];
			}

			PBuffer* Data() {
				return data_;
			}
		};
	}
}