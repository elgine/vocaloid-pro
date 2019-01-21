#pragma once
#include "stdafx.h"
#ifdef _WIN32 || _WIN64
#include "ffmpeg_io.hpp"
#else
#include "wav.hpp"
#endif
#include "../utility/buffer.hpp"
#include "../utility/path.hpp"
namespace vocaloid {
	namespace io {
		int64_t ReadFileBuffer(const char* source, AudioFormat *format, Buffer<char> *buffer) {
			AudioFileReader *reader = nullptr;
#ifdef _WIN32 || _WIN64
			reader = new FFmpegFileReader();
#else
			if (GetExtension(source) != ".wav") {
				throw "Don't support audio file except 'wav'";
			}
			reader = new WAVReader();
#endif
			auto ret = reader->Open(source);
			if (ret < 0)return ret;
			AudioFormat f = {};
			reader->GetFormat(&f);
			format->sample_rate = f.sample_rate;
			format->channels = f.channels;
			format->bits = f.bits;
			format->block_align = f.block_align;

			buffer->Alloc(reader->FileLength());

			auto frame_size = DEFAULT_FRAME_SIZE;
			int64_t size = 0;
			int64_t sum = 0;
			auto temp = new char[frame_size];

			while (!reader->End() || reader->CapableToRead(frame_size)) {
				size = reader->ReadData(temp, frame_size);
				if (size > 0) {
					buffer->Add(temp, size);
					sum += size;
				}
			}
			reader->Flush(temp, size);
			if (size > 0) {
				buffer->Add(temp, size);
				sum += size;
			}
			reader->Stop();
			reader->Dispose();
			delete[] temp;
			temp = nullptr;
			return sum;
		}
	}
}