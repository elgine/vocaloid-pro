#pragma once
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/read_short_file.hpp"
#include <future>
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::io;
void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto convolution = new ConvolutionNode(context);
	auto buffer = new Buffer<char>();
	auto format = new AudioFormat();
	auto fut = async(ReadShortFile, "G:\\Projects\\VSC++\\vocaloid\\samples\\radio.wav", format, buffer);
	fut.get();
	auto kernel = new AudioFrame();
	kernel->FromBuffer(buffer, format->bits, format->channels);
	auto mono = kernel->Channel(0)->Data();
	convolution->kernel_.assign(mono.begin(), mono.end());

	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	context->Connect(source, convolution);
	context->Connect(convolution, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Close();
}