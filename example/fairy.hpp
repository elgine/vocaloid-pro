#include "../vocaloid/phase_vocoder_node.hpp"
#include "../vocaloid/flanger.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/auto_wah.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;
using namespace vocaloid::effect;

void Run() {
	auto context = new AudioContext();
	auto pitch_shifter = new PhaseVocoderNode(context);
	pitch_shifter->pitch_ = 1.5f;

	auto flanger = new Flanger(context);
	flanger->SetOptions({
		5.0f,
		0.0f,
		0.001f,
		0.2f
	});

	auto auto_wah = new AutoWah(context);
	auto_wah->SetOptions({
		2.5f,
		3.0f,
		5.0f
	});

	auto amplify = new GainNode(context, 2.41f);
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	context->Connect(source, pitch_shifter);
	context->Connect(pitch_shifter, flanger->input_);
	
	context->Connect(flanger->output_, player);
	/*context->Connect(flanger->output_, auto_wah->input_);
	context->Connect(auto_wah->output_, amplify);
	context->Connect(amplify, player);*/
	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}