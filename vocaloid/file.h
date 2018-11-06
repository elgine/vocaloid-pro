#pragma once
#include "stdafx.h"
namespace vocaloid {
	namespace io {
		// Common audio file format
		struct AudioFormat {
			uint32_t sample_rate;
			uint16_t bits;
			uint16_t channels;
			uint16_t block_align;
		};

		class AudioFileReader{
		public:
			virtual int64_t ReadData(char* data, uint64_t length) = 0;
			virtual int16_t Open(const char* input_path) = 0;
			virtual void Close() = 0;
			virtual bool IsEnd() = 0;
			virtual void Flush(char* data, uint64_t& length) = 0;
			virtual uint64_t Seek(uint64_t pos) = 0;
			virtual AudioFormat Format() = 0;
		};

		class AudioFileWriter {
		public:
			virtual int16_t Open(const char* output_path, uint32_t sample_rate, uint16_t bits, uint16_t channels) = 0;
			virtual int64_t WriteData(const char* bytes, uint64_t byte_length) = 0;
			virtual void Close() = 0;
		};
	}
}