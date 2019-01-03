#include "../vocaloid/Jungle.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::composite;
void Run() {
	
	auto context = new AudioContext();
	context->SetOutput(OutputType::PLAYER);
	auto source = new FileReaderNode(context);
	source->Open("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	auto compressor = new DynamicsCompressorNode(context);

	auto dee = new Jungle(context);
	dee->SetPitchOffset(-0.1f);
	auto deep = new Jungle(context);
	deep->SetPitchOffset(-0.2f);
	auto deeper = new Jungle(context);
	deeper->SetPitchOffset(-0.4f);
	auto deeperer = new Jungle(context);
	deeperer->SetPitchOffset(-0.8f);

	dee->Start();
	deep->Start();
	deeper->Start();
	deeperer->Start();

	context->Connect(source, dee->input_);
	context->Connect(source, deep->input_);
	context->Connect(source, deeper->input_);
	context->Connect(source, deeperer->input_);

	context->Connect(dee->output_, compressor);
	context->Connect(deep->output_, compressor);
	context->Connect(deeper->output_, compressor);
	context->Connect(deeperer->output_, compressor);
	context->Connect(compressor, context->Destination());

	context->Prepare();
	context->Start();
	getchar();
	context->Dispose();
}