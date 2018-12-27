#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/gain_node.hpp"
#include "effects.h"
using namespace vocaloid::node;
namespace effect {
	
	class Effect {
	protected:
		Effects id_;
		AudioContext *ctx_;
		GainNode *gain_;
	public:
		explicit Effect(AudioContext *ctx) {
			ctx_ = ctx;
			gain_ = new GainNode(ctx);
		}

		void SetGain(float gain) {
			gain_->gain_->value_ = gain;
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
}