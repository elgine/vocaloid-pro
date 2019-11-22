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
	context->SetOutput(OutputType::RECORDER);
	auto dest = (FileWriterNode*)context->Destination();
	dest->SetPath("C:\\Users\\Admin\\Desktop\\out.mp3");
	auto source = new FileReaderNode(context);
	source->Open("D:\\Projects\\vocaloid\\samples\\example.mp3");

	auto segments = new int[6]{
		0, 4000,
		20000, 30000,
		55000, 60000
	};
	source->StartWithSegments(segments, 3);
	auto convolution = new ConvolutionNode(context);
	auto channel_data = new AudioChannel();
	auto buffer = new Buffer<char>();
	auto format = new AudioFormat();

	ReadFileBuffer("D:\\Projects\\vocaloid\\samples\\muffler.wav", format, buffer);
	channel_data->FromBuffer(buffer, format->bits, format->channels);
	convolution->kernel_ = channel_data;

	auto amplify = new GainNode(context, 1.0f);

	context->Connect(source, convolution);
	context->Connect(convolution, amplify);
	context->Connect(amplify, context->Destination());

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
	context->Dispose();
}