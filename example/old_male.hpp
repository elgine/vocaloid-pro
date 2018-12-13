#include "../vocaloid/Jungle.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/vibrato.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;

void Run() {
	auto context = new AudioContext();
	auto low_pass = new BiquadNode(context);
	low_pass->frequency_->value_ = 2000;
	auto pitch_shifter = new Jungle(context);
	pitch_shifter->SetPitchOffset(-0.2);
	auto vibrato = new Vibrato(context);
	vibrato->SetOptions({
		0.03f,
		0.002f,
		5.0f,
	});
	auto amplify = new GainNode(context, 1.8);

	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	context->Connect(source, low_pass);
	context->Connect(low_pass, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, vibrato->input_);
	context->Connect(vibrato->output_, amplify);
	context->Connect(amplify, player);
	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}