#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "../vocaloid/jungle.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto compressor = new DynamicsCompressorNode(context);
	auto lowpass = new BiquadNode(context);
	lowpass->frequency_->value_ = 3000;
	auto phase_vocoder = new PhaseVocoderNode(context);
	phase_vocoder->pitch_ = 0.8;
	auto convolution = new ConvolutionNode(context);
	auto buffer = new Buffer<char>();
	auto format = new io::AudioFormat();
	io::ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\parking.wav", format, buffer);
	auto channel_data = new AudioChannel();
	channel_data->FromBuffer(buffer, format->bits, format->channels);
	convolution->SetKernel(channel_data->Channel(0)->Data(), channel_data->Size());
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	//context->Connect(source, phase_vocoder);
	//context->Connect(phase_vocoder, compressor);
	//context->Connect(phase_vocoder, convolution);
	//context->Connect(compressor, lowpass);
	//context->Connect(convolution, lowpass);
	//context->Connect(lowpass, player);
	context->Connect(source, convolution);
	context->Connect(convolution, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();

}