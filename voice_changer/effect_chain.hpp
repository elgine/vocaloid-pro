#pragma once
#include "effect.hpp"
#include "effect_factory.hpp"
namespace effect {

	class EffectChain {
	private:
		BaseAudioContext* ctx_;
		vector<Effect*> effects_;
	public:

		explicit EffectChain(BaseAudioContext *ctx) {
			ctx_ = ctx;
		}

		void SetChain(Effects* ids) {
			
		}

		void SetOptions(float *options, float* option_counts) {
			
		}

		void Dispose() {}
	};
}