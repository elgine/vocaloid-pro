#include "../vocaloid/pitch_shifter.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;

void Run() {
	auto context = new AudioContext();
	auto low_pass = new BiquadNode(context);
	low_pass->frequency_->value_ = 2000;
	auto pitch_shifter = new PitchShifter(context);
	pitch_shifter->SetPitchOffset(-0.2188);
	auto equalizer = new Equalizer3Band(context);
	equalizer->SetOptions({
		100, -4,
		900, 1,
		2500, 2
	});
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	context->Connect(source, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, equalizer->input_);
	context->Connect(equalizer->output_, player);
	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}