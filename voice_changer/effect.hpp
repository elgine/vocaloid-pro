#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/gain_node.hpp"
using namespace vocaloid::node;
namespace effect {
	struct EffectOptions {
		float gain;
	};
	
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

		void SetOptions(EffectOptions options) {
			gain_->gain_->value_ = options.gain;
		}

		virtual AudioNode* Input() {
			return gain_;
		}

		virtual AudioNode* Output() {
			return gain_;
		}

		virtual void Start() {}

		virtual void Dispose() {}
	};
}