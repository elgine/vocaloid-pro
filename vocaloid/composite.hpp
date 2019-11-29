#pragma once
#include "gain_node.hpp"
namespace vocaloid {
	namespace composite {
		using namespace vocaloid;
		using namespace vocaloid::node;
		class Composite {
		protected:
			BaseAudioContext *ctx_;

			static float MIX_DEFAULT;
			static float MIX_MIN;
			static float MIX_MAX;
		public:		
			GainNode *input_;
			GainNode *dry_;
			GainNode *wet_;
			GainNode *output_;

			explicit Composite(BaseAudioContext *ctx) {
				ctx_ = ctx;
				input_ = new GainNode(ctx);
				dry_ = new GainNode(ctx);
				wet_ = new GainNode(ctx);
				output_ = new GainNode(ctx);
				ctx->Connect(dry_, output_);
				ctx->Connect(wet_, output_);
				CrossFade(1.0);
			}

			virtual void Dispose() {
				input_->Dispose();
				delete input_;
				input_ = nullptr;
				dry_->Dispose();
				delete dry_;
				dry_ = nullptr;
				wet_->Dispose();
				delete wet_;
				wet_ = nullptr;
				output_->Dispose();
				delete output_;
				output_ = nullptr;
			}

			virtual void Start() {}

			virtual void Resume() {}

			virtual void Stop(){}

			void CrossFade(float v) {
				v = Clamp(MIX_MIN, MIX_MAX, v);
				auto gain1 = cosf(v * 0.5 * M_PI);
				auto gain2 = cosf((1.0 - v) * 0.5 * M_PI);
				dry_->gain_->value_ = gain1;
				wet_->gain_->value_ = gain2;
			}

			BaseAudioContext* Context() {
				return ctx_;
			}
		};

		float Composite::MIX_DEFAULT = 0.0;
		float Composite::MIX_MIN = 0.0;
		float Composite::MIX_MAX = 1.0;
	}
}