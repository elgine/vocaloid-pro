#include "../vocaloid/jungle.hpp"
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
	auto high_pass = new BiquadNode(context);
	high_pass->type_ = BIQUAD_TYPE::HIGH_PASS;
	high_pass->frequency_->value_ = 1250;
	auto pitch_shifter = new Jungle(context);
	pitch_shifter->SetPitchOffset(0.2005f);
	auto vibrato = new Vibrato(context);
	vibrato->SetOptions({
		0.05f,
		0.001f,
		7.2f,
	});
	auto amplify = new GainNode(context, 1.5);
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	
	context->Connect(source, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, high_pass);
	context->Connect(high_pass, vibrato->input_);
	context->Connect(vibrato->output_, amplify);
	context->Connect(amplify, player);
	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}