#pragma once
#include "../vocaloid/biquad_node.hpp"
#include "../vocaloid/jungle.hpp"
#include "effect.hpp"
#include "effects.h"
using namespace vocaloid::composite;

namespace effect {
	class Male : public Effect {
	private:
		BiquadNode *lowpass_;
		Jungle *jungle_;
	public:

		static float LOWPASS_FREQ_DEFAULT;
		static float LOWPASS_FREQ_MIN;
		static float LOWPASS_FREQ_MAX;

		static float PITCH_OFFSET_DEFAULT;
		static float PITCH_OFFSET_MIN;
		static float PITCH_OFFSET_MAX;

		explicit Male(AudioContext* ctx) :Effect(ctx) {
			id_ = Effects::MALE;
			lowpass_ = new BiquadNode(ctx);
			lowpass_->frequency_->value_ = 4500;
			jungle_ = new Jungle(ctx);
			jungle_->SetPitchOffset(-0.1);

			ctx->Connect(lowpass_, jungle_->input_);
			ctx->Connect(jungle_->output_, gain_);
		}

		AudioNode *Input() {
			return lowpass_;
		}

		void Start() override {
			jungle_->Start();
		}

		void Dispose() override {
			lowpass_->Dispose();
			delete lowpass_;
			lowpass_ = nullptr;

			jungle_->Dispose();
			delete jungle_;
			jungle_ = nullptr;

			Effect::Dispose();
		}
	};

	float Male::LOWPASS_FREQ_DEFAULT = 2000.0f;
	float Male::LOWPASS_FREQ_MIN = 500.0f;
	float Male::LOWPASS_FREQ_MAX = 6000.0f;

	float Male::PITCH_OFFSET_DEFAULT = -0.1f;
	float Male::PITCH_OFFSET_MIN = -0.8f;
	float Male::PITCH_OFFSET_MAX = 0.0f;
}