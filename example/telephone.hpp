#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/player_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");

	auto b1 = new BiquadNode(context);
	b1->frequency_->value_ = 2000;
	auto b2 = new BiquadNode(context);
	b2->frequency_->value_ = 2000;
	auto b3 = new BiquadNode(context);
	b3->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	b3->frequency_->value_ = 500;
	auto b4 = new BiquadNode(context);
	b4->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	b4->frequency_->value_ = 500;
	auto compressor = new DynamicsCompressorNode(context);

	context->Connect(source, b1);
	context->Connect(b1, b2);
	context->Connect(b2, b3);
	context->Connect(b3, b4);
	context->Connect(b4, compressor);
	context->Connect(compressor, player);

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}