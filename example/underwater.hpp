#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/auto_wah.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::io;
using namespace vocaloid::composite;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);
	auto input_gain = new GainNode(context, 0.5f);

	auto filter = new BiquadNode(context);
	filter->frequency_->value_ = 500.0f;
	auto compressor = new DynamicsCompressorNode(context);

	auto underwater = new FileReaderNode(context);
	underwater->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\underwater.mp3");
	underwater->Start(0);
	underwater->loop_ = true;
	auto underwater_gain = new GainNode(context, 0.3f);

	auto wahwah = new AutoWah(context);
	wahwah->SetOptions({
		10,
		3.5,
		20
	});

	context->Connect(source, input_gain);
	context->Connect(input_gain, wahwah->input_);
	context->Connect(wahwah->output_, filter);
	context->Connect(filter, compressor);
	context->Connect(compressor, player);

	context->Connect(underwater, underwater_gain);
	context->Connect(underwater_gain, compressor);

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Close();
}