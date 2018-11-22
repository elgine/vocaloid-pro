#pragma once
#include "audio_context.hpp"
#include "effect.hpp"
#include "oscillator_node.hpp"
#include "gain_node.hpp"
#include <rapidjson.h>
using namespace rapidjson;
namespace vocaloid {
	namespace effect {
		using namespace vocaloid;
		using namespace vocaloid::node;

		class WahWah : public Effect {
		public:
			OscillatorNode *lfo_;

			explicit WahWah(AudioContext *ctx) :Effect(ctx) {
				input_ = new GainNode(ctx);
				lfo_ = new OscillatorNode(ctx);
				output_ = new GainNode(ctx);
			}

			void SetParam() {
				
			}
		};
	}
}