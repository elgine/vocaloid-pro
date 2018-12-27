#pragma once
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include <future>
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::io;
void Run() {
	auto context = new AudioContext();
	context->SetOutput(OutputType::PLAYER);
	auto convolution = new ConvolutionNode(context);
	auto buffer = new Buffer<char>();
	auto format = new AudioFormat();
	auto fut = async(ReadFileBuffer, "G:\\Projects\\VSC++\\vocaloid\\samples\\radio.wav", format, buffer);
	fut.get();
	auto kernel = new AudioChannel();
	kernel->FromBuffer(buffer, format->bits, format->channels);
	convolution->kernel_ = kernel;

	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	context->Connect(source, convolution);
	context->Connect(convolution, context->Destination());

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Dispose();
}