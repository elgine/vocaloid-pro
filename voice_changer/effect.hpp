#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/gain_node.hpp"
#include "effects.h"
using namespace vocaloid;
using namespace vocaloid::node;
namespace effect {
	
	class Effect {
	protected:
		Effects id_;
		AudioContext *ctx_;
		GainNode *gain_;
	public:

		static float GAIN_DEFAULT;
		static float GAIN_MIN;
		static float GAIN_MAX;

		explicit Effect(AudioContext *ctx) {
			ctx_ = ctx;
			gain_ = new GainNode(ctx, 1.0f);
		}

		void SetGain(float gain) {
			gain_->gain_->value_ = Clamp(GAIN_MIN, GAIN_MAX, gain);
		}

		virtual void SetOptions(float* options, int option_count) {
			
		}

		Effects Id() {
			return id_;
		}

		virtual AudioNode* Input() {
			return gain_;
		}

		virtual AudioNode* Output() {
			return gain_;
		}

		virtual void Start() {}

		virtual void Dispose() {
			gain_->Dispose();
			delete gain_;
			gain_ = nullptr;
		}
	};

	float Effect::GAIN_DEFAULT = 0.8f;
	float Effect::GAIN_MIN = 0.0f;
	float Effect::GAIN_MAX = 4.0f;
}