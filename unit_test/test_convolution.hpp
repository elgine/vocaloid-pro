#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../vocaloid/biquad_node.hpp"
using namespace vocaloid;
using namespace vocaloid::io;
using namespace vocaloid::node;
using namespace vocaloid::dsp;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	auto convolver = new ConvolutionNode(context);
	auto buf = new Buffer<char>();
	auto format = new AudioFormat();
	ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\large-wide-echo-hall.wav", format, buf);
	auto kernel = new AudioChannel();
	kernel->FromBuffer(buf, format->bits, format->channels);
	convolver->kernel_ = kernel;

	context->Connect(source, convolver);
	context->Connect(convolver, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Dispose();
}