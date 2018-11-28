#include "../vocaloid/biquad.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/pitch_shifter.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	auto equalizer = new Equalizer3Band(context);
	equalizer->SetOptions({
		100,
		-1,
		900,
		1,
		2500,
		-30
	});
	auto amplify = new GainNode(context, 1.92);
	auto pitch_shifter = new PitchShifter(context);
	pitch_shifter->SetPitchOffset(0.5);
	
	context->Connect(source, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, equalizer->input_);
	context->Connect(equalizer->output_, amplify);
	context->Connect(amplify, player);

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}