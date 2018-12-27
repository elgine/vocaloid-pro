//
// Created by Elgine on 2018/10/31.
//
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/player_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	context->SetOutput(OutputType::PLAYER);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	auto delay = new DelayNode(context, 0.01);
	auto osc1 = new OscillatorNode(context);
	osc1->SetWaveform(50, dsp::WAVEFORM_TYPE::SAWTOOTH);
	auto osc2 = new OscillatorNode(context);
	osc2->SetWaveform(1000, dsp::WAVEFORM_TYPE::SAWTOOTH);
	auto osc3 = new OscillatorNode(context);
	osc3->SetFrequency(50);
	auto gain = new GainNode(context, 0.004);

	context->Connect(osc1, gain);
	context->Connect(osc2, gain);
	context->Connect(osc3, gain);
	context->Connect(gain, delay->delay_time_);
	context->Connect(source, delay);
	context->Connect(delay, context->Destination());

	source->Start(0);
	osc1->Start();
	osc2->Start();
	osc3->Start();

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
}