//
// Created by Elgine on 2018/10/31.
//
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/biquad_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	context->SetPlayerMode(44100, 2);
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
	osc1->Connect(gain);
	osc2->Connect(gain);
	osc3->Connect(gain);
	gain->Connect(delay->delay_time_);
	source->Connect(delay);
	delay->Connect(context->GetDestination());

	context->Setup();
	context->Start();
	getchar();
	context->Stop();
}