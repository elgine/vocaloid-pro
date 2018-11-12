//
// Created by Elgine on 2018/10/30.
//
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/player_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	auto delay = new DelayNode(context, 0.01);
	auto osc = new OscillatorNode(context);
	osc->SetFrequency(700.0f);
	auto osc_gain = new GainNode(context, 0.004f);
	auto biquad = new BiquadNode(context);
	biquad->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	biquad->frequency_->value_ = 695;
	context->Connect(osc, osc_gain);
	context->Connect(osc_gain, delay->delay_time_);
	context->Connect(source, delay);
	context->Connect(delay, biquad);
	context->Connect(biquad, player);
	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
}