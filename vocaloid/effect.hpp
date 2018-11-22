#pragma once
#include "audio_context.hpp"
namespace vocaloid {
	namespace effect {
		using namespace vocaloid;
		using namespace vocaloid::node;
		class Effect {
		protected:
			AudioContext *ctx_;
		public:		
			AudioProcessorUnit *input_;
			AudioProcessorUnit *output_;

			explicit Effect(AudioContext *ctx) {
				ctx_ = ctx;
			}

			AudioContext* Context() {
				return ctx_;
			}
		};
	}
}