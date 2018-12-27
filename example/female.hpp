#include "../vocaloid/biquad.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/jungle.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::dsp;
using namespace vocaloid::composite;

void Run() {
	auto context = new AudioContext();
	auto high_pass = new BiquadNode(context);
	high_pass->type_ = BIQUAD_TYPE::HIGH_PASS;
	high_pass->frequency_->value_ = 120;
	context->SetOutput(OutputType::PLAYER);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	auto pitch_shifter = new Jungle(context);
	pitch_shifter->SetPitchOffset(0.6);

	context->Connect(source, pitch_shifter->input_);
	context->Connect(pitch_shifter->output_, high_pass);
	context->Connect(high_pass, context->Destination());


	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Dispose();
}