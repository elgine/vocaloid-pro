#include "../vocaloid/ffmpeg_io.hpp"
#include "../vocaloid/maths.hpp"
#include "../vocaloid/pcm_player.hpp"
using namespace vocaloid;
using namespace vocaloid::io;

void TestWriter() {
	auto frequency = 440;
	auto sample_rate = 44100;
	auto bits = 16;
	auto channels = 1;
	auto duration = 2;
	auto frame_size_per_time = sample_rate / frequency;
	auto frame_size = sample_rate * duration;
	auto byte_len = frame_size *  bits / 8 * channels;
	auto bytes = new char[byte_len];
	float max = powf(2.0f, bits - 1) - 1;
	auto depth = bits / 8;
	for (int i = 0; i < frame_size; i++) {
		float value = sin(M_PI_2 * (i % frame_size_per_time) / frame_size_per_time);
		value = Clamp(-1.0f, 1.0f, value);
		auto v = (long)(value * max);
		for (int k = 0; k < depth; k++) {
			bytes[i * depth + k] = (char)((v >> 8 * k) & 0xFF);
		}
	}

	auto writer = new FFmpegFileWriter();
	writer->Open("output.mp3", sample_rate, bits, channels);
	writer->WriteData(bytes, byte_len);
	writer->Flush();
	writer->Close();
}

void TestReader() {
	auto player = new PCMPlayer();
	auto reader = new FFmpegFileReader();
	reader->Open("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	auto format = reader->Format();
	player->Open(format.sample_rate, format.bits, format.channels);
	auto size = 4096;
	auto bytes = new char[size];
	while (!reader->IsEnd() || reader->CapableToRead(size)) {
		auto act_size = reader->ReadData(bytes, size);
		if (act_size > 0)player->Push(bytes, act_size);
	}
	reader->Close();
	player->Flush();
	player->Close();
}