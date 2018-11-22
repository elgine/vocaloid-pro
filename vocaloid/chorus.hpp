#pragma once
#include "effect.hpp"
#include "gain_node.hpp"
#include "delay_node.hpp"
namespace vocaloid {
	namespace effect {
		using namespace vocaloid;
		using namespace vocaloid::node;

		class Chorus : public Effect {
		public:
			GainNode *attenuator_;
			DelayNode *delay_l_;
			DelayNode *delay_r_;
			GainNode *feedback_lr_;
			GainNode *feedback_rl_;

			explicit Chorus(AudioContext *ctx):Effect(ctx){
				input_ = new GainNode(ctx);
				attenuator_ = new GainNode(ctx);
				delay_l_ = new DelayNode(ctx);
				delay_r_ = new DelayNode(ctx);
				output_ = new GainNode(ctx);
				feedback_lr_ = new GainNode(ctx);
				feedback_rl_ = new GainNode(ctx);
				output_ = new GainNode(ctx);
			}
		};
	}
}