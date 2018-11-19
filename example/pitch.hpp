#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/pitch_shifter.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/file_writer_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
void Run() {
	auto context = new AudioContext();
	auto writer = new FileWriterNode(context);
	writer->SetPath("ugly-mix.mp3");
	auto player = new PlayerNode(context);
	auto pitch_shifter = new PitchShifter(context);
	pitch_shifter->pitch_ = 1.22f;
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\ugly.mp3");

	context->Connect(source, pitch_shifter);
	context->Connect(pitch_shifter, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
	context->Close();
}