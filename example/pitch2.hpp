#pragma once
#include "./jungle.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
void Run() {
	auto context = new AudioContext();
	auto jungle = new Jungle(context);
	auto compressor = new DynamicsCompressorNode(context);
	auto player = new PlayerNode(context);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\meet.mp3");

	context->Connect(source, jungle->input_);
	context->Connect(jungle->output_, compressor);
	jungle->SetPitchOffset(0.34);
	context->Connect(compressor, player);

	source->Start(0);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}