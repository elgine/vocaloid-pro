#pragma once
#include <stdint.h>
namespace vocaloid {
	namespace io {
		class Player {
		public:
			// Open device with specific audio format
			virtual int Open(int32_t sample_rate, int16_t bits, int16_t nChannels) = 0;

			// Push an audio frame data
			virtual int Push(const char* buf, size_t size) = 0;

			// Flush buffers left
			virtual int Flush() = 0;

			// Buffer size has played 
			virtual int64_t Played() = 0;

			// Clear buffers and records
			virtual void Clear() = 0;

			// Stop playing
			virtual void Stop() = 0;

			// Dispose
			virtual void Dispose() = 0;

			// Resume
			virtual void Resume() = 0;
		};
	}
}