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

	auto osc1 = new OscillatorNode(context);
	osc1->SetFrequency(10);
	osc1->SetWaveformType(WAVEFORM_TYPE::SAWTOOTH);

	auto osc2 = new OscillatorNode(context);
	osc2->SetFrequency(50);
	osc2->SetWaveformType(WAVEFORM_TYPE::SAWTOOTH);

	auto osc3 = new OscillatorNode(context);
	osc3->SetFrequency(30);
	osc3->SetWaveformType(WAVEFORM_TYPE::SAWTOOTH);

	auto osc_gain = new GainNode(context);
	osc_gain->gain_->value_ = 0.007;

	auto osc_gain2 = new GainNode(context);
	osc_gain2->gain_->value_ = 0.007;

	auto delay = new DelayNode(context);
	delay->delay_time_->value_ = 0.01;

	auto delay2 = new DelayNode(context);
	delay2->delay_time_->value_ = 0.01;

	auto filter = new BiquadNode(context);
	filter->type_ = BIQUAD_TYPE::LOW_PASS;
	filter->frequency_->value_ = 2000;

	auto convolver = new ConvolutionNode(context);
	auto buf = new Buffer<char>();
	auto format = new AudioFormat();
	ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\parking.wav", format, buf);
	auto kernel = new AudioChannel();
	kernel->FromBuffer(buf, format->bits, format->channels);
	convolver->kernel_ = kernel;

	auto compressor = new DynamicsCompressorNode(context);
	auto compressor2 = new DynamicsCompressorNode(context);
	auto compressor3 = new DynamicsCompressorNode(context);

	context->Connect(osc1, osc_gain);
	context->Connect(osc2, osc_gain);
	context->Connect(osc_gain, delay->delay_time_);

	context->Connect(source, compressor2);
	context->Connect(compressor2, delay);
	context->Connect(delay, compressor3);
	context->Connect(compressor3, filter);
	context->Connect(filter, convolver);
	context->Connect(convolver, player);

	/*context->Connect(osc3, osc_gain2);
	context->Connect(osc_gain2, delay2->delay_time_);*/

	auto no_conv_gain = new GainNode(context, 0.9f);
	context->Connect(filter, no_conv_gain);
	context->Connect(no_conv_gain, player);

	osc1->Start();
	osc2->Start();
	osc3->Start();

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Close();
}