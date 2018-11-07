//
// Created by Elgine on 2018/11/1.
//
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/wave_shaper_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

void MakeDistortionCurve(int amount, uint32_t samples, vector<float> &curve) {
	auto deg = M_PI / 180;
	float x;
	for (int i = 0; i < samples; ++i) {
		x = i * 2.0f / samples - 1;
		curve[i] = float((3 + amount) * x * 20 * deg / (M_PI + amount * abs(x)));
	}
}

void Run() {
	auto context = new AudioContext();
	context->SetPlayerMode(44100, 2);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	auto b1 = new BiquadNode(context);
	b1->frequency_->value_ = 1300;
	auto wave_shaper = new WaveShaperNode(context);
	uint32_t curve_len = 44100;
	vector<float> curve(curve_len);
	MakeDistortionCurve(50, curve_len, curve);
	wave_shaper->SetCurve(curve, curve_len);
	auto b2 = new BiquadNode(context);
	b2->frequency_->value_ = 2000;
	auto b3 = new BiquadNode(context);
	b3->frequency_->value_ = 2000;
	auto b4 = new BiquadNode(context);
	b4->frequency_->value_ = 500;
	b4->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	auto b5 = new BiquadNode(context);
	b5->frequency_->value_ = 500;
	b5->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	auto compressor = new DynamicsCompressorNode(context);
	source->Connect(b1);
	b1->Connect(wave_shaper);
	wave_shaper->Connect(b2);
	b2->Connect(b3);
	b3->Connect(b4);
	b4->Connect(b5);
	b5->Connect(compressor);
	compressor->Connect(context->GetDestination());
	context->Setup();
	context->Start();
	getchar();
	context->Stop();
}
