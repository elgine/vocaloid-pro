#pragma once
#include "../vocaloid/biquad.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "./jungle.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;
void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto filter = new BiquadNode(context);
	filter->type_ = BIQUAD_TYPE::ALL_PASS;
	filter->frequency_->value_ = 4000.0f;
	auto pitch_shifter = new Jungle(context);
	pitch_shifter->SetPitchOffset(-0.45);
	auto amplify = new GainNode(context, 1.8f);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\kaluli.mp3");

	context->Connect(source, filter);
	context->Connect(filter, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, amplify);
	context->Connect(amplify, player);

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}