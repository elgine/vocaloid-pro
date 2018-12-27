#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/player_node.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/wave_shaper_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

float* MakeDistortionCurve(int16_t amount = 100, int32_t sample_rate = 44100, float sec = 1.0f) {
	auto n_samples = sample_rate * sec;
	float *curve = new float[n_samples];
	auto deg = M_PI / 180;
	for (auto i = 0; i < n_samples; i++) {
		float x = i * 2 / n_samples - 1;
		curve[i] = (3 + amount) * x * 20 * deg / (M_PI + amount * abs(x));
	}
	return curve;
}

void Run() {
	auto context = new AudioContext();
	context->SetOutput(OutputType::PLAYER);
	auto source = new FileReaderNode(context);
	source->SetPath("G:\\Projects\\cpp\\vocaloid\\samples\\speech.wav");
	source->Start(0);

	auto compressor = new DynamicsCompressorNode(context);

	auto wave_shaper = new WaveShaperNode(context);
	auto curve = MakeDistortionCurve();
	wave_shaper->SetCurve(curve, 44100);

	auto lpf1 = new BiquadNode(context);
	lpf1->frequency_->value_ = 2000.0f;

	auto lpf2 = new BiquadNode(context);
	lpf2->frequency_->value_ = 2000.0f;

	auto hpf1 = new BiquadNode(context);
	hpf1->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	hpf1->frequency_->value_ = 500;

	auto hpf2 = new BiquadNode(context);
	hpf2->type_ = dsp::BIQUAD_TYPE::HIGH_PASS;
	hpf2->frequency_->value_ = 500;

	context->Connect(source, lpf1);
	context->Connect(lpf1, lpf2);
	context->Connect(lpf2, hpf1);
	context->Connect(hpf1, hpf2);
	context->Connect(hpf2, wave_shaper);
	context->Connect(wave_shaper, compressor);
	context->Connect(compressor, context->Destination());

	context->Prepare();
	context->Start();
	getchar();
	context->Stop();
}