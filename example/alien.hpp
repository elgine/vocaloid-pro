#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	context->SetPlayerMode(44100, 2);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	auto delay = new DelayNode(context, 0.05f, 0.1f);
	auto osc = new OscillatorNode(context);
	osc->SetWaveform(5.0f, dsp::WAVEFORM_TYPE::SINE);
	auto osc_gain = new GainNode(context, 0.05f);
	osc->Connect(osc_gain);
	osc_gain->Connect(delay->delay_time_);
	source->Connect(delay);
	delay->Connect(context->GetDestination());

	context->Setup();
	context->Start();
	getchar();
	context->Stop();
}