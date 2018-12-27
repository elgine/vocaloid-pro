#pragma once
#include "stdafx.h"
#include <fstream>
#include "file.h"
namespace vocaloid {
	namespace io {
		struct WAV_HEADER {
			char riff[5];
			int32_t size0;
			char wave[5];
			char fmt[5];
			int32_t size1;
			int16_t format_tag;
			int16_t channels;
			int32_t samples_per_sec;
			int32_t bytes_per_sec;
			int16_t block_align;
			int16_t bits_per_sec;
			int32_t extra_size;
			uint8_t *extra;
			char data[5];
			int32_t size2;
		};

		class WAVWriter : public AudioFileWriter {
		private:
			WAV_HEADER header_;
			ofstream out_;
			size_t data_chunk_pos_;

			template <typename Word>
			std::ostream& Write(std::ostream& outs, Word value, unsigned size = sizeof(Word)) {
				for (; size; --size, value >>= 8)
					outs.put(static_cast <char> (value & 0xFF));
				return outs;
			}
		public:
			int16_t Open(const char* output_path, int32_t sample_rate, int16_t bits, int16_t channels) override {
				int16_t block_align = bits / 8 * channels;
				int32_t bytes_per_sec = block_align * sample_rate;
				header_ = {
					{ 'R', 'I', 'F', 'F', '\0' },
					36,
					{ 'W', 'A', 'V', 'E', '\0' },
					{ 'f', 'm', 't', ' ', '\0' },
					16,
					1,
					channels,
					sample_rate,
					bytes_per_sec,
					block_align,
					bits,
					0,
					{},
					{ 'd', 'a', 't', 'a', '\0' },
					0
				};
				data_chunk_pos_ = 0;

				out_.open(output_path, ios::binary);
				out_.write((char*)&header_.riff, 4);
				out_.write((char*)&header_.size0, sizeof header_.size0);
				out_.write((char*)&header_.wave, 4);
				out_.write((char*)&header_.fmt, 4);
				out_.write((char*)&header_.size1, sizeof header_.size1);
				out_.write((char*)&header_.format_tag, sizeof header_.format_tag);
				out_.write((char*)&header_.channels, sizeof header_.channels);
				out_.write((char*)&header_.samples_per_sec, sizeof header_.samples_per_sec);
				out_.write((char*)&header_.bytes_per_sec, sizeof header_.bytes_per_sec);
				out_.write((char*)&header_.block_align, sizeof header_.block_align);
				out_.write((char*)&header_.bits_per_sec, sizeof header_.bits_per_sec);
				out_.write((char*)&header_.data, 4);
				out_.write((char*)&header_.size2, sizeof header_.size2);
				data_chunk_pos_ = out_.tellp();
				return 0;
			}

			int64_t WriteData(const char* bytes, int64_t byte_length) override {
				out_.write(bytes, byte_length);
				header_.size0 += byte_length;
				header_.size2 += byte_length;
				return header_.size2;
			}

			void Dispose() override {
				out_.seekp(data_chunk_pos_ - 4);
				Write(out_, header_.size2, 4);
				out_.seekp(0 + 4);
				Write(out_, header_.size0, 4);
				out_.close();
			}

			WAV_HEADER GetHeader() {
				return header_;
			}
		};

		class WAVReader : public AudioFileReader {
		private:
			WAV_HEADER header_;
			AudioFormat format_;
			ifstream in_;
			int64_t pos_;
			bool has_extra_data_;
		public:
			int16_t Open(const char* source_path) override {
				has_extra_data_ = false;
				header_ = {
					{ ' ', ' ', ' ', ' ', '\0' },
					36,
					{ ' ', ' ', ' ', ' ', '\0' },
					{ ' ', ' ', ' ', ' ', '\0' },
					16,
					1,
					0,
					0,
					0,
					0,
					0,
					0,
					{},
					{ ' ', ' ', ' ', ' ', '\0' },
					0
				};
				in_.open(source_path, ios::in | ios::binary);
				in_.read((char*)&header_.riff, 4);
				if (strcmp(header_.riff, "RIFF") != 0) {
					return -1;
				}
				in_.read((char*)&header_.size0, sizeof(int32_t));
				in_.read((char*)&header_.wave, 4);
				if (strcmp(header_.wave, "WAVE") != 0) {
					return -1;
				}
				in_.read((char*)&header_.fmt, 4);
				if (strcmp(header_.fmt, "fmt ") != 0) {
					return -1;
				}
				in_.read((char*)&header_.size1, sizeof(int32_t));
				in_.read((char*)&header_.format_tag, sizeof(int16_t));
				in_.read((char*)&header_.channels, sizeof(int16_t));
				in_.read((char*)&header_.samples_per_sec, sizeof(int32_t));
				in_.read((char*)&header_.bytes_per_sec, sizeof(int32_t));
				in_.read((char*)&header_.block_align, sizeof(int16_t));
				in_.read((char*)&header_.bits_per_sec, sizeof(int16_t));
				in_.read((char*)&header_.data, 4);
				format_.sample_rate = header_.samples_per_sec;
				format_.bits = header_.bits_per_sec;
				format_.channels = header_.channels;
				format_.block_align = format_.channels * format_.bits / 8;
				if (strcmp(header_.data, "data") != 0) {
					has_extra_data_ = true;
					in_.read((char*)&header_.extra_size, sizeof(int32_t));
					header_.extra = new uint8_t[header_.extra_size];
					in_.read((char*)header_.extra, header_.extra_size);
					in_.read((char*)&header_.data, 4);
					if (strcmp(header_.data, "data") != 0)return -1;
					printf("%s\n", header_.extra);
				}
				in_.read((char*)&header_.size2, sizeof(int32_t));
				pos_ = 0;
				return 0;
			}

			bool CapableToRead(int64_t len) override {
				return header_.size2 - pos_ >= len;
			}

			int32_t GetHeaderLength() {
				int32_t length = 44;
				if (has_extra_data_) {
					length += 4 + header_.extra_size;
				}
				return length;
			}

			int64_t FileLength() override {
				return header_.size2;
			}

			int64_t Seek(int64_t time) override {
				auto pos = time * 0.001 * header_.samples_per_sec * header_.block_align;
				if (pos < header_.size2 && pos > 0) {
					pos_ = pos;
					in_.seekg(pos_ + GetHeaderLength());
				}
				return pos_;
			}

			int64_t ReadData(char *bytes, int64_t byte_length) override {
				if (IsEnd())return 0;
				byte_length = min(header_.size2 - pos_, byte_length);
				in_.read(bytes, byte_length);
				pos_ += byte_length;
				return byte_length;
			}

			void Stop() override {
				
			}

			void Clear() override {}

			void Flush(char *bytes, int64_t &byte_length) override {
				byte_length = header_.size2 - pos_;
				in_.read(bytes, byte_length);
				pos_ += byte_length;
			}

			bool IsEnd() override {
				return in_.eof() || header_.size2 - pos_ <= 0;
			}

			void Dispose() override {
				in_.close();
			}

			WAV_HEADER GetHeader() {
				return header_;
			}

			AudioFormat Format() override {
				return format_;
			}
		};
	}
}