#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
void Run() {
	auto context = new AudioContext();
	auto player = new PlayerNode(context);
	auto source1 = new FileReaderNode(context);
	auto source2 = new FileReaderNode(context);
	auto source3 = new FileReaderNode(context);
	auto source4 = new FileReaderNode(context);
	auto source5 = new FileReaderNode(context);
	auto source6 = new FileReaderNode(context);
	auto source7 = new FileReaderNode(context);

	auto gain1 = new GainNode(context);
	auto gain2 = new GainNode(context);
	auto gain3 = new GainNode(context);
	auto gain4 = new GainNode(context);
	auto gain5 = new GainNode(context);
	auto gain6 = new GainNode(context);
	auto gain7 = new GainNode(context); 
	auto gain8 = new GainNode(context);
	auto gain9 = new GainNode(context);
	auto gain10 = new GainNode(context);

	auto delay1 = new DelayNode(context);
	auto delay2 = new DelayNode(context);

	auto dynamicCompressor = new DynamicsCompressorNode(context);

	context->Connect(source1, gain1);
	context->Connect(source2, gain2);
	context->Connect(source3, gain3);
	context->Connect(source4, gain4);
	context->Connect(source5, gain5);

	context->Connect(gain1, gain6);
	context->Connect(gain2, gain6);
	context->Connect(gain3, gain7);
	context->Connect(gain4, gain7);

	context->Connect(gain5, delay1);
	context->Connect(gain5, delay2);
	context->Connect(gain6, delay1->delay_time_);
	context->Connect(gain7, delay2->delay_time_);

	context->Connect(delay1, gain8);
	context->Connect(source6, gain8->gain_);
	context->Connect(delay2, gain9);
	context->Connect(source7, gain9->gain_);
	context->Connect(gain8, gain10);
	context->Connect(gain9, gain10);
	context->Connect(gain10, dynamicCompressor);
	context->Connect(dynamicCompressor, player);

	context->Prepare();
	context->Start();
	getchar();
	context->Close();
}