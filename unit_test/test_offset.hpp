#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/audio_context.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
void Run() {
	auto ctx = new AudioContext();
	auto source = new FileReaderNode(ctx);
	auto player = new PlayerNode(ctx);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");

	ctx->Connect(source, context->Destination());

	source->Start(0, 5000);

	ctx->Prepare();
	ctx->Start();
	getchar();
	ctx->Close();
}