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
#include "../vocaloid/file_writer_node.hpp"
#include "../vocaloid/file_writer_node.hpp"
using namespace vocaloid;
using namespace vocaloid::io;
using namespace vocaloid::node;
using namespace vocaloid::dsp;

void Run() {
	auto context = new AudioContext();

	auto filter = new BiquadNode(context);
	filter->type_ = BIQUAD_TYPE::HIGH_SHELF;
	filter->frequency_->value_ = 1000;
	filter->gain_->value_ = 10;

	auto compressor = new DynamicsCompressorNode(context);
	compressor->threshold_ = -50;
	compressor->ratio_ = 16;

	auto delay = new DelayNode(context);
	delay->delay_time_->value_ = 0.01;

	auto osc = new OscillatorNode(context);
	osc->SetWaveformType(WAVEFORM_TYPE::SAWTOOTH);
	osc->SetFrequency(50);
	auto osc_gain = new GainNode(context);
	osc_gain->gain_->value_ = 0.004;

	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	auto writer = new FileWriterNode(context);
	writer->SetPath("C:\\Users\\Elgine\\Desktop\\out.mp3");

	auto convolver = new ConvolutionNode(context);
	auto buf = new Buffer<char>();
	auto format = new AudioFormat();
	ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\large-wide-echo-hall.wav", format, buf);
	auto channel = new AudioChannel();
	channel->FromBuffer(buf, format->bits, format->channels);
	convolver->kernel_ = channel;

	auto convolver_gain = new GainNode(context);
	convolver_gain->gain_->value_ = 0.5;
	
	auto fire = new FileReaderNode(context);
	fire->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\fire.mp3");
	auto fire_gain = new GainNode(context);
	fire_gain->gain_->value_ = 0.1;
	fire->loop_ = true;

	auto filter2 = new BiquadNode(context);
	filter2->type_ = dsp::BIQUAD_TYPE::LOW_PASS;
	filter2->frequency_->value_ = 2000;

	auto no_conv_gain = new GainNode(context);
	no_conv_gain->gain_->value_ = 0.9;

	auto player = new PlayerNode(context);

	auto amplify = new GainNode(context, 2.5f);

	context->Connect(osc, osc_gain);
	context->Connect(osc_gain, delay->delay_time_);

	context->Connect(source, delay);
	context->Connect(delay, convolver);

	context->Connect(convolver, convolver_gain);
	context->Connect(convolver_gain, filter);	
	context->Connect(filter, compressor);
	context->Connect(compressor, amplify);

	context->Connect(delay, filter2);
	context->Connect(filter2, filter);
	context->Connect(filter, no_conv_gain);
	context->Connect(no_conv_gain, compressor);

	context->Connect(fire, fire_gain);
	context->Connect(fire_gain, amplify);

	context->Connect(amplify, writer);

	source->Start(0);
	fire->Start(0);
	osc->Start();

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Close();
}