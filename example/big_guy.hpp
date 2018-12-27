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
	context->SetOutput(OutputType::PLAYER);
	auto lowpass = new BiquadNode(context);
	lowpass->frequency_->value_ = 4000;
	auto pitch_shifter = new PhaseVocoderNode(context);
	pitch_shifter->pitch_ = 0.88f;
	auto amplify = new GainNode(context, 1.0f);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	context->Connect(source, lowpass);
	context->Connect(lowpass, pitch_shifter);
	context->Connect(pitch_shifter, amplify);
	context->Connect(amplify, context->Destination());

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Dispose();
}