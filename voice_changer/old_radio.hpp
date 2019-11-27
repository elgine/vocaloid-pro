#pragma once
#include "effect.hpp"
#include "effects.h"
#include "../vocaloid/maths.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/buffer_node.hpp"
#include "../vocaloid/distortion.hpp"
#include "../vocaloid/convolver.hpp"

namespace effect {
	class OldRadio : public Effect {
	private:
		OscillatorNode *osc_;
		GainNode* osc_gain_;
		DelayNode* delay_;
		BufferNode* noise_;
		GainNode* noise_wobble_;
		ConvolutionNode *convolver_;
		DistortionNode *waveshaper_;

		AudioChannel* kernel_;
		AudioChannel* noise_source_;
	public:

		static float OSC_DEFAULT;
		static float OSC_MIN;
		static float OSC_MAX;

		static float OSC_GAIN_DEFAULT;
		static float OSC_GAIN_MIN;
		static float OSC_GAIN_MAX;

		static float DELAY_DEFAULT;
		static float DELAY_MIN;
		static float DELAY_MAX;

		static float NOISE_WOBBLE_DEFAULT;
		static float NOISE_WOBBLE_MIN;
		static float NOISE_WOBBLE_MAX;

		explicit OldRadio(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::OLD_RADIO;
			convolver_ = new ConvolutionNode(ctx);
			kernel_ = new AudioChannel();
			LoadKernel(IDR_PORTABLE_RADIO, L"wav", kernel_);
			convolver_->kernel_ = kernel_;
			osc_ = new OscillatorNode(ctx);
			osc_gain_ = new GainNode(ctx);
			delay_ = new DelayNode(ctx);
			noise_ = new BufferNode(ctx);
			noise_->Loop(true);
			noise_source_ = new AudioChannel(1, 32768, ctx_->SampleRate());
			for (auto i = 0; i < 32768; i += 3) {
				auto v = rand() / 32768.0 * 2.0;
				noise_source_->Channel(0)->Data()[i] = v;
				noise_source_->Channel(0)->Data()[i + 1] = v;
				noise_source_->Channel(0)->Data()[i + 2] = v;
			}
			noise_->SetBuffer(noise_source_);
			waveshaper_ = new DistortionNode(ctx);
			waveshaper_->gain_ = 1.0f;
			noise_wobble_ = new GainNode(ctx);

			ctx_->Connect(noise_, noise_wobble_->gain_);
			ctx_->Connect(noise_wobble_, delay_);
			ctx_->Connect(osc_, osc_gain_);
			ctx_->Connect(osc_gain_, delay_->delay_time_);
			ctx_->Connect(delay_, convolver_);
			ctx_->Connect(convolver_, waveshaper_);
			ctx_->Connect(waveshaper_, gain_);
		}

		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				osc_->SetFrequency(Clamp(OSC_MIN, OSC_MAX, options[0]));
			}
			if (option_count > 1) {
				osc_gain_->gain_->value_ = Clamp(OSC_GAIN_MIN, OSC_GAIN_MAX, options[1]);
			}
			if (option_count > 2) {
				delay_->delay_time_->value_ = Clamp(DELAY_MIN, DELAY_MAX, options[2]);
			}
			if (option_count > 3) {
				noise_wobble_->gain_->value_ = Clamp(NOISE_WOBBLE_MIN, NOISE_WOBBLE_MAX, options[3]);
			}
			if (option_count > 4) {
				SetGain(options[2]);
			}
		}

		void Start() override {
			osc_->Start();
			noise_->Start();
		}

		void Resume() override {
			osc_->Resume();
			noise_->Resume();
		}

		void Dispose() override {
			if (kernel_) {
				kernel_->Dispose();
				delete kernel_;
				kernel_ = nullptr;
			}
			if (osc_) {
				osc_->Dispose();
				delete osc_; osc_ = nullptr;
			}
			if (osc_gain_) {
				osc_gain_->Dispose();
				delete osc_gain_;
				osc_gain_ = nullptr;
			}
			if (delay_) {
				delay_->Dispose();
				delete delay_;
				delay_ = nullptr;
			}
			if (waveshaper_) {
				waveshaper_->Dispose();
				delete waveshaper_;
				waveshaper_ = nullptr;
			}
			if (noise_wobble_) {
				noise_wobble_->Dispose();
				delete noise_wobble_;
				noise_wobble_ = nullptr;
			}
			if (convolver_) {
				convolver_->Dispose();
				delete convolver_;
				convolver_ = nullptr;
			}
			if (noise_) {
				noise_->Dispose();
				delete noise_;
				noise_ = nullptr;
			}
			Effect::Dispose();
		}

		AudioNode* Input() {
			return noise_wobble_;
		}
	};

	float OldRadio::OSC_DEFAULT = 0.2f;
	float OldRadio::OSC_MIN = 0;
	float OldRadio::OSC_MAX = 1;

	float OldRadio::OSC_GAIN_DEFAULT = 0.001f;
	float OldRadio::OSC_GAIN_MIN = 0.0;
	float OldRadio::OSC_GAIN_MAX = 0.1f;

	float OldRadio::DELAY_DEFAULT = 0.01;
	float OldRadio::DELAY_MIN = 0.005;
	float OldRadio::DELAY_MAX = 0.1;

	float OldRadio::NOISE_WOBBLE_DEFAULT = 0.5;
	float OldRadio::NOISE_WOBBLE_MIN = 0.1;
	float OldRadio::NOISE_WOBBLE_MAX = 1.0;
}