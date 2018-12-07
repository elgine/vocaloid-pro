#pragma once
#include "./effect.hpp"
#include "./reverb_node.hpp"
namespace vocaloid {
	namespace effect {
		using namespace node;

		class Reverb : public Effect {
		public:
			struct ReverbOptions {
				float decay;
				bool reverse;
				float time;
			};

			ReverbNode *reverb_;

			Reverb(AudioContext *ctx):Effect(ctx){
				reverb_ = new ReverbNode(ctx);
				ctx->Connect(input_, reverb_);
				ctx->Connect(reverb_, wet_);
			}

			void SetOptions(ReverbOptions options) {
				reverb_->decay_ = options.decay;
				reverb_->reverse_ = options.reverse;
				reverb_->time_ = options.time;
			}
		};
	}
}