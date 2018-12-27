#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/phase_vocoder_node.hpp"
#include "../vocaloid/equalizer_3_band.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace vocaloid::composite;

void Run() {
	auto ctx = new AudioContext();
	auto gain = new GainNode(ctx);
	gain->gain_->value_ = 2.5f;
	auto pitch_shifter = new PhaseVocoderNode(ctx);
	pitch_shifter->pitch_ = 1.5f;
	auto equalizer = new Equalizer3Band(ctx);
	auto player = new PlayerNode(ctx);
	auto source = new FileReaderNode(ctx);
	auto high_pass = new BiquadNode(ctx);
	high_pass->frequency_->value_ = 120;
	high_pass->type_ = BIQUAD_TYPE::HIGH_PASS;
	source->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	source->Start(0);
	ctx->Connect(source, equalizer->input_);
	ctx->Connect(equalizer->output_, pitch_shifter);
	ctx->Connect(pitch_shifter, high_pass);
	ctx->Connect(high_pass, gain);
	ctx->Connect(gain, player);
	ctx->Prepare();
	ctx->Start();
	getchar();
	ctx->Close();
}