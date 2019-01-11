#pragma once
#include <stdint.h>
namespace vocaloid {
	namespace io {
		class Player {
		public:
			virtual int Open(int32_t sample_rate, int16_t bits, int16_t nChannels) = 0;
			virtual int Push(const char* buf, size_t size) = 0;
			virtual int Flush() = 0;
			virtual int64_t Played() = 0;
			virtual void Clear() = 0;
			virtual void Stop() = 0;
			virtual void Dispose() = 0;
			virtual void Resume() = 0;
		};
	}
}