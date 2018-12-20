#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../vocaloid/gain_node.hpp"

using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::io;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);
	auto convolution = new ConvolutionNode(context);
	auto channel_data = new AudioChannel();
	auto buffer = new Buffer<char>();
	auto format = new AudioFormat();

	ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\large-long-echo-hall.wav", format, buffer);
	channel_data->FromBuffer(buffer, format->bits, format->channels);
	convolution->kernel_ = channel_data;

	auto amplify = new GainNode(context, 1.0f);

	context->Connect(source, convolution);
	context->Connect(convolution, amplify);
	context->Connect(amplify, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}