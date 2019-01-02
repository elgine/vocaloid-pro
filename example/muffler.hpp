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
	context->SetOutput(OutputType::PLAYER);
	auto source = new FileReaderNode(context);
	source->Open("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	auto segments = new int64_t*[3]{
		new int64_t[2]{0, 4000},
		new int64_t[2]{ 20000, 30000},
		new int64_t[2]{ 55000, 60000}
	};
	source->StartWithSegments(segments, 3);
	source->loop_ = true;
	auto convolution = new ConvolutionNode(context);
	auto channel_data = new AudioChannel();
	auto buffer = new Buffer<char>();
	auto format = new AudioFormat();

	ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\muffler.wav", format, buffer);
	channel_data->FromBuffer(buffer, format->bits, format->channels);
	convolution->kernel_ = channel_data;

	auto amplify = new GainNode(context, 1.0f);

	context->Connect(source, convolution);
	context->Connect(convolution, amplify);
	context->Connect(amplify, context->Destination());

	context->Prepare();
	context->Start();
	getchar();
	context->Dispose();
}