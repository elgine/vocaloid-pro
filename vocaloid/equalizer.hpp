#pragma once
#include "audio_context.hpp"
#include "biquad_node.hpp"
#include "effect.hpp"
namespace vocaloid {
	namespace effect {
		using namespace vocaloid;
		using namespace vocaloid::node;

		class Equalizer : public Effect {
		public:
			explicit Equalizer(AudioContext *ctx):Effect(ctx){
				input_ = new BiquadNode(ctx);
				output_ = new BiquadNode(ctx);
				ctx_->Connect(input_, output_);
			}

			void SetParams() {
				
			}
		};
	}
}