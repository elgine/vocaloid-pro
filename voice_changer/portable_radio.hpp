#pragma once
#include "effect.hpp"
#include "effects.h"
#include "../vocaloid/maths.hpp"
#include "../vocaloid/oscillator_node.hpp"
#include "../vocaloid/delay_node.hpp"
#include "../vocaloid/gain_node.hpp"
#include "../vocaloid/squeak.hpp"
#include "../vocaloid/distortion.hpp"

namespace effect {
	class PortableRadio : public Effect {
	private:
		OscillatorNode *osc_;
		GainNode *osc_gain_;
		DelayNode *delay_;
		SqueakNode *noise_;
		DistortionNode *waveshaper_;
	public:

		static float OSC_DEFAULT;
		static float OSC_MIN;
		static float OSC_MAX;

		static float OSC_GAIN_DEFAULT;
		static float OSC_GAIN_MIN;
		static float OSC_GAIN_MAX;

		static float SQUEAK_DEPTH_DEFAULT;
		static float SQUEAK_DEPTH_MIN;
		static float SQUEAK_DEPTH_MAX;

		static float SQUEAK_MASTER_GAIN_DEFAULT;
		static float SQUEAK_MASTER_GAIN_MIN;
		static float SQUEAK_MASTER_GAIN_MAX;

		static float SQUEAK_SAMPLE_PERIOD_DEFAULT;
		static float SQUEAK_SAMPLE_PERIOD_MIN;
		static float SQUEAK_SAMPLE_PERIOD_MAX;

		static float SQUEAK_NOISE_GAIN_DEFAULT;
		static float SQUEAK_NOISE_GAIN_MIN;
		static float SQUEAK_NOISE_GAIN_MAX;

		static float SQUEAK_MOD_PERIOD_DEFAULT;
		static float SQUEAK_MOD_PERIOD_MIN;
		static float SQUEAK_MOD_PERIOD_MAX;

		static float SQUEAK_GAIN_DEFAULT;
		static float SQUEAK_GAIN_MIN;
		static float SQUEAK_GAIN_MAX;

		explicit PortableRadio(BaseAudioContext *ctx) :Effect(ctx) {
			id_ = Effects::PORTABLE_RADIO;
			osc_ = new OscillatorNode(ctx);
			osc_gain_ = new GainNode(ctx);
			delay_ = new DelayNode(ctx);
			noise_ = new SqueakNode(ctx);
			waveshaper_ = new DistortionNode(ctx);
			ctx_->Connect(osc_, osc_gain_);
			ctx_->Connect(osc_gain_, delay_->delay_time_);
			ctx_->Connect(delay_, waveshaper_);
			ctx_->Connect(waveshaper_, noise_);
			ctx_->Connect(noise_, gain_);

			osc_->SetFrequency(OSC_DEFAULT);
			osc_gain_->gain_->value_ = OSC_GAIN_DEFAULT;
			delay_->delay_time_->value_ = 0.02;
		}

		void SetOptions(float *options, int option_count) override {
			if (option_count > 0) {
				osc_->SetFrequency(Clamp(OSC_MIN, OSC_MAX, options[0]));
			}
			if (option_count > 1) {
				osc_gain_->gain_->value_ = Clamp(OSC_GAIN_MIN, OSC_GAIN_MAX, options[1]);
			}
			if (option_count > 2) {
				noise_->depth_ = Clamp(SQUEAK_DEPTH_MIN, SQUEAK_DEPTH_MAX, options[2]);
			}
			if (option_count > 3) {
				noise_->master_gain_ = Clamp(SQUEAK_MASTER_GAIN_MIN, SQUEAK_MASTER_GAIN_MAX, options[3]);
			}
			if (option_count > 4) {
				noise_->sample_period_ = Clamp(SQUEAK_SAMPLE_PERIOD_MIN, SQUEAK_SAMPLE_PERIOD_MAX, options[4]);
			}
			if (option_count > 2) {
				SetGain(options[2]);
			}
		}

		void Start() override {
			osc_->Start();
		}

		void Resume() override {
			osc_->Resume();
		}

		void Dispose() override {
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
			if (noise_) {
				noise_->Dispose();
				delete noise_;
				noise_ = nullptr;
			}
			Effect::Dispose();
		}

		AudioNode* Input() {
			return delay_;
		}
	};

	float PortableRadio::OSC_DEFAULT = 0.2f;
	float PortableRadio::OSC_MIN = -0;
	float PortableRadio::OSC_MAX = 5;

	float PortableRadio::OSC_GAIN_DEFAULT = 0.001f;
	float PortableRadio::OSC_GAIN_MIN = 0.0;
	float PortableRadio::OSC_GAIN_MAX = 0.1f;

	float PortableRadio::SQUEAK_DEPTH_DEFAULT = 1.0;
	float PortableRadio::SQUEAK_DEPTH_MIN = 0.1;
	float PortableRadio::SQUEAK_DEPTH_MAX = 2.0;

	float PortableRadio::SQUEAK_MASTER_GAIN_DEFAULT = 0.5;
	float PortableRadio::SQUEAK_MASTER_GAIN_MIN = 0.1;
	float PortableRadio::SQUEAK_MASTER_GAIN_MAX = 1.0;

	float PortableRadio::SQUEAK_SAMPLE_PERIOD_DEFAULT = 7;
	float PortableRadio::SQUEAK_SAMPLE_PERIOD_MIN = 3;
	float PortableRadio::SQUEAK_SAMPLE_PERIOD_MAX = 10;

	float PortableRadio::SQUEAK_NOISE_GAIN_DEFAULT = 0.1;
	float PortableRadio::SQUEAK_NOISE_GAIN_MIN = 0;
	float PortableRadio::SQUEAK_NOISE_GAIN_MAX = 1.0;

	float PortableRadio::SQUEAK_MOD_PERIOD_DEFAULT = 0.04;
	float PortableRadio::SQUEAK_MOD_PERIOD_MIN = 0.01;
	float PortableRadio::SQUEAK_MOD_PERIOD_MAX = 0.1;

	float PortableRadio::SQUEAK_GAIN_DEFAULT = 0.05;
	float PortableRadio::SQUEAK_GAIN_MIN = 0.01;
	float PortableRadio::SQUEAK_GAIN_MAX = 0.1;
}