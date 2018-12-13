#pragma once
#include "../vocaloid/biquad.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;
void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto filter = new BiquadNode(context);
	filter->type_ = BIQUAD_TYPE::LOW_PASS;
	filter->frequency_->value_ = 6000.0f;
	auto pitch_shifter = new PhaseVocoderNode(context);
	pitch_shifter->pitch_ = 0.93f;
	auto amplify = new GainNode(context, 1.8f);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	context->Connect(source, pitch_shifter);
	/*context->Connect(filter, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, amplify);*/
	//context->Connect(filter, pitch_shifter);
	context->Connect(pitch_shifter, player);
	//context->Connect(amplify, player);

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}