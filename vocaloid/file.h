#pragma once
#include "stdafx.h"
namespace vocaloid {
	namespace io {
		// Common audio file format
		struct AudioFormat {
			int32_t sample_rate;
			int16_t bits;
			int16_t channels;
			int16_t block_align;
		};

		class AudioFileReader{
		public:
			virtual int64_t ReadData(char* data, int64_t length) = 0;
			virtual int16_t Open(const char* input_path) = 0;
			virtual void Dispose() = 0;
			virtual void Stop() = 0;
			virtual void Clear() = 0;
			virtual bool End() = 0;
			virtual void Flush(char* data, int64_t& length) = 0;
			virtual int64_t Seek(int64_t pos) = 0;
			virtual AudioFormat Format() = 0;
			virtual int64_t FileLength() = 0;
			virtual int64_t Duration() = 0;
			virtual bool CapableToRead(int64_t len) = 0;
		};

		class AudioFileWriter {
		public:
			virtual int16_t Open(const char* output_path, int32_t sample_rate, int16_t bits, int16_t channels) = 0;
			virtual int64_t WriteData(const char* bytes, int64_t byte_length) = 0;
			virtual void Dispose() = 0;
		};
	}
}