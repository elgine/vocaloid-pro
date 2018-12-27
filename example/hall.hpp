#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::io;

void Run() {
	auto context = new AudioContext();
	context->SetOutput(OutputType::PLAYER);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	auto convolver = new ConvolutionNode(context);
	auto buf = new Buffer<char>();
	auto format = new AudioFormat();
	ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\church.wav", format, buf);
	auto channel = new AudioChannel();
	channel->FromBuffer(buf, format->bits, format->channels);
	convolver->kernel_ = channel;
	context->Connect(source, convolver);
	context->Connect(convolver, context->Destination());

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Dispose();
}