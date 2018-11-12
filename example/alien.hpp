#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/file_writer_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	auto delay = new DelayNode(context, 0.05f, 0.1f);
	auto osc = new OscillatorNode(context);
	osc->SetFrequency(5.0f);
	auto osc_gain = new GainNode(context, 0.05f);

	context->Connect(osc, osc_gain);
	context->Connect(osc_gain, delay->delay_time_);
	context->Connect(source, delay);
	context->Connect(delay, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}