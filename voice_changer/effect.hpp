#pragma once
#include "../vocaloid/gain_node.hpp"
#include "effects.h"
using namespace vocaloid;
using namespace vocaloid::node;
namespace effect {
	
	class Effect {
	protected:
		Effects id_;
		BaseAudioContext *ctx_;
		GainNode *gain_;
	public:

		static float GAIN_DEFAULT;
		static float GAIN_MIN;
		static float GAIN_MAX;

		explicit Effect(BaseAudioContext *ctx) {
			ctx_ = ctx;
			gain_ = new GainNode(ctx, GAIN_DEFAULT);
		}

		virtual void Dispose() {
			gain_->Dispose();
			delete gain_;
			gain_ = nullptr;
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

		virtual void Resume() {}

		virtual void Start() {}
	};

	float Effect::GAIN_DEFAULT = 0.8f;
	float Effect::GAIN_MIN = 0.0f;
	float Effect::GAIN_MAX = 4.0f;
}