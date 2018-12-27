#include "../vocaloid/pitch_shifter_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	context->SetOutput(OutputType::PLAYER)
	auto pitch_shifter = new PitchShifterNode(context);
	pitch_shifter->pitch_ = 0.8;
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	context->Connect(source, pitch_shifter);
	context->Connect(pitch_shifter, context->Destination());

	context->Prepare();
	context->Start();
	getchar();
	context->Dispose();
}