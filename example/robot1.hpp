//
// Created by Elgine on 2018/10/30.
//
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	context->SetPlayerMode(44100, 2);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	auto delay = new DelayNode(context, 0.01);
	auto osc = new OscillatorNode(context);
	osc->SetFrequency(700);
	auto osc_gain = new GainNode(context, 0.004);
	auto biquad = new BiquadNode(context);
	biquad->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	biquad->frequency_->value_ = 695;

	osc->Connect(osc_gain);
	osc_gain->Connect(delay->delay_time_);

	source->Connect(delay);
	delay->Connect(biquad);
	biquad->Connect(context->GetDestination());

	context->Setup();
	context->Start();
	getchar();
	context->Stop();
}