#pragma once
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/convolution_node.hpp"
#include "../vocaloid/dynamic_compressor_node.hpp"
#include "../vocaloid/biquad_node.hpp"
#include "../utility/buffer.hpp"
#include "../vocaloid/read_file_buffer.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "effects.h"
#include "effect.hpp"

namespace effect {

	class Balrog : public Effect {
	private:
		BiquadNode *filter_;
		DynamicsCompressorNode *compressor_;
		DelayNode *delay_;
		ConvolutionNode *convolver_;
		OscillatorNode *osc_;
		GainNode *osc_gain_;
		GainNode *convolver_gain_;
		GainNode *fire_gain_;
		GainNode *no_conv_gain_;
		FileReaderNode *fire_;
		BiquadNode *filter2_;
	public:
		static float LFO_FREQ_DEFAULT;
		static float LFO_FREQ_MIN;
		static float LFO_FREQ_MAX;

		static float LFO_GAIN_DEFAULT;
		static float LFO_GAIN_MIN;
		static float LFO_GAIN_MAX;

		static float DELAY_DEFAULT;
		static float DELAY_MIN;
		static float DELAY_MAX;

		static float HIGHSHELF_FREQ_DEFAULT;
		static float HIGHSHELF_FREQ_MIN;
		static float HIGHSHELF_FREQ_MAX;

		static float HIGHSHELF_GAIN_DEFAULT;
		static float HIGHSHELF_GAIN_MIN;
		static float HIGHSHELF_GAIN_MAX;

		static float COMPRESSOR_THRESHOLD_DEFAULT;
		static float COMPRESSOR_THRESHOLD_MIN;
		static float COMPRESSOR_THRESHOLD_MAX;

		static float COMPRESSOR_RATIO_DEFAULT;
		static float COMPRESSOR_RATIO_MIN;
		static float COMPRESSOR_RATIO_MAX;

		static float ECHO_GAIN_DEFAULT;
		static float ECHO_GAIN_MIN;
		static float ECHO_GAIN_MAX;

		static float MAIN_GAIN_DEFAULT;
		static float MAIN_GAIN_MIN;
		static float MAIN_GAIN_MAX;

		static float FIRE_GAIN_DEFAULT;
		static float FIRE_GAIN_MIN;
		static float FIRE_GAIN_MAX;

		explicit Balrog(AudioContext *ctx) : Effect(ctx) {
			id_ = Effects::BALROG;
			filter_ = new BiquadNode(ctx);
			filter_->type_ = BIQUAD_TYPE::HIGH_SHELF;
			filter_->frequency_->value_ = 1000;
			filter_->gain_->value_ = 10;

			compressor_ = new DynamicsCompressorNode(ctx);
			compressor_->threshold_ = -50;
			compressor_->ratio_ = 16;

			delay_ = new DelayNode(ctx);
			delay_->delay_time_->value_ = 0.01;

			osc_ = new OscillatorNode(ctx);
			osc_->SetWaveformType(WAVEFORM_TYPE::SAWTOOTH);
			osc_->SetFrequency(50);
			osc_gain_ = new GainNode(ctx);
			osc_gain_->gain_->value_ = 0.004;

			convolver_ = new ConvolutionNode(ctx);
			auto buf = new vocaloid::Buffer<char>();
			auto format = new vocaloid::io::AudioFormat();
			ReadFileBuffer("G:\\Projects\\VSC++\\vocaloid\\samples\\large-wide-echo-hall.wav", format, buf);
			auto channel = new AudioChannel();
			channel->FromBuffer(buf, format->bits, format->channels);
			convolver_->kernel_ = channel;
			buf->Dispose();

			convolver_gain_ = new GainNode(ctx);
			convolver_gain_->gain_->value_ = 0.5;

			fire_ = new FileReaderNode(ctx);
			fire_->SetPath("G:\\Projects\\VSC++\\vocaloid\\samples\\fire.mp3");
			fire_gain_ = new GainNode(ctx);
			fire_gain_->gain_->value_ = 0.1;
			fire_->loop_ = true;

			filter2_ = new BiquadNode(ctx);
			filter2_->type_ = vocaloid::dsp::BIQUAD_TYPE::LOW_PASS;
			filter2_->frequency_->value_ = 2000;

			no_conv_gain_ = new GainNode(ctx);
			no_conv_gain_->gain_->value_ = 2.0;

			ctx->Connect(osc_, osc_gain_);
			ctx->Connect(osc_gain_, delay_->delay_time_);
			ctx->Connect(delay_, convolver_);
			ctx->Connect(convolver_, convolver_gain_);
			ctx->Connect(convolver_gain_, filter_);
			ctx->Connect(filter_, compressor_);
			ctx->Connect(compressor_, gain_);
			ctx->Connect(delay_, filter2_);
			ctx->Connect(filter2_, filter_);
			ctx->Connect(filter_, no_conv_gain_);
			ctx->Connect(no_conv_gain_, compressor_);
			ctx->Connect(fire_, fire_gain_);
			ctx->Connect(fire_gain_, gain_);
		}

		void Start() override {
			fire_->Start(0);
			osc_->Start();
		}

		void Dispose() override {
			filter_->Dispose();
			delete filter_; filter_ = nullptr;

			compressor_->Dispose();
			delete compressor_; compressor_ = nullptr;

			delay_->Dispose();
			delete delay_; delay_ = nullptr;

			convolver_->Dispose();
			delete convolver_; convolver_ = nullptr;

			osc_->Dispose();
			delete osc_; osc_ = nullptr;

			osc_gain_->Dispose();
			delete osc_gain_; osc_gain_ = nullptr;

			convolver_gain_->Dispose();
			delete convolver_gain_; convolver_gain_ = nullptr;

			fire_gain_->Dispose();
			delete fire_gain_; fire_gain_ = nullptr;

			no_conv_gain_->Dispose();
			delete no_conv_gain_; no_conv_gain_ = nullptr;

			fire_->Dispose();
			delete fire_; fire_ = nullptr;

			filter2_->Dispose();
			delete filter2_; filter2_ = nullptr;

			Effect::Dispose();
		}

		AudioNode *Input() {
			return delay_;
		}
	};

	float Balrog::LFO_FREQ_DEFAULT = 50.0f;
	float Balrog::LFO_FREQ_MIN = 20.0f;
	float Balrog::LFO_FREQ_MAX = 70.0f;

	float Balrog::LFO_GAIN_DEFAULT = 0.004f;
	float Balrog::LFO_GAIN_MIN = 0.001f;
	float Balrog::LFO_GAIN_MAX = 0.1f;

	float Balrog::DELAY_DEFAULT = 0.01f;
	float Balrog::DELAY_MIN = 0.005f;
	float Balrog::DELAY_MAX = 0.02f;

	float Balrog::HIGHSHELF_FREQ_DEFAULT = 1000.0f;
	float Balrog::HIGHSHELF_FREQ_MIN = 500.0f;
	float Balrog::HIGHSHELF_FREQ_MAX = 2000.0f;

	float Balrog::HIGHSHELF_GAIN_DEFAULT = 10.0f;
	float Balrog::HIGHSHELF_GAIN_MIN = -40.0f;
	float Balrog::HIGHSHELF_GAIN_MAX = 40.0f;

	float Balrog::COMPRESSOR_THRESHOLD_DEFAULT = -50.0f;
	float Balrog::COMPRESSOR_THRESHOLD_MIN = -100.0f;
	float Balrog::COMPRESSOR_THRESHOLD_MAX = 0.0f;

	float Balrog::COMPRESSOR_RATIO_DEFAULT = 16.0f;
	float Balrog::COMPRESSOR_RATIO_MIN = 1;
	float Balrog::COMPRESSOR_RATIO_MAX = 20;

	float Balrog::ECHO_GAIN_DEFAULT = 0.5f;
	float Balrog::ECHO_GAIN_MIN = 0.0f;
	float Balrog::ECHO_GAIN_MAX = 1.0f;

	float Balrog::MAIN_GAIN_DEFAULT = 2.0f;
	float Balrog::MAIN_GAIN_MIN = 0.5f;
	float Balrog::MAIN_GAIN_MAX = 4.0f;

	float Balrog::FIRE_GAIN_DEFAULT = 0.3f;
	float Balrog::FIRE_GAIN_MIN = 0.0f;
	float Balrog::FIRE_GAIN_MAX = 1.0f;
}