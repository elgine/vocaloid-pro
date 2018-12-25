#pragma once
#include "audio_context.hpp"
#include "gain_node.hpp"
namespace vocaloid {
	namespace composite {
		using namespace vocaloid;
		using namespace vocaloid::node;
		class Composite {
		protected:
			AudioContext *ctx_;
		public:		
			GainNode *input_;
			GainNode *dry_;
			GainNode *wet_;
			GainNode *output_;

			explicit Composite(AudioContext *ctx) {
				ctx_ = ctx;
				input_ = new GainNode(ctx);
				dry_ = new GainNode(ctx);
				wet_ = new GainNode(ctx);
				output_ = new GainNode(ctx);
				ctx->Connect(dry_, output_);
				ctx->Connect(wet_, output_);
				CrossFade(1.0);
			}

			virtual void Start() {}

			void CrossFade(float v) {
				auto gain1 = cosf(v * 0.5 * M_PI);
				auto gain2 = cosf((1.0 - v) * 0.5 * M_PI);
				dry_->gain_->value_ = gain1;
				wet_->gain_->value_ = gain2;
			}

			AudioContext* Context() {
				return ctx_;
			}
		};
	}
}