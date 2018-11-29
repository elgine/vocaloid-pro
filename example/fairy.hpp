#include "../vocaloid/pitch_shifter.hpp"
#include "../vocaloid/flanger.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/auto_wah.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;

void Run() {
	auto context = new AudioContext();
	auto pitch_shifter = new PitchShifter(context);
	pitch_shifter->SetPitchOffset(0.2f);

	auto flanger = new Flanger(context);
	flanger->SetOptions({
		0.001f,
		0.35f,
		0.17f,
		950
	});

	auto auto_wah = new AutoWah(context);
	/*auto_wah->SetOptions({
		
	});*/

	auto amplify = new GainNode(context, 2.41f);
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");


	context->Connect(source, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, flanger->input_);
	context->Connect(flanger->output_, auto_wah->input_);
	context->Connect(auto_wah->output_, amplify);
	context->Connect(amplify, player);
	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}