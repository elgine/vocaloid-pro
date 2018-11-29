#include "effect.hpp"
#include "wave_shaper_node.hpp"
#include "biquad_node.hpp"

namespace vocaloid {
	namespace effect {
		using namespace vocaloid::node;
		class AutoWah : public Effect {
		public:
			struct AutoWahOptions {
				float aw_follower_frequency;
				float awr_depth;
				float aw_filter_Q;
			};

			WaveShaperNode *wave_shaper_;
			BiquadNode *aw_follower_;
			GainNode *aw_depth_;
			BiquadNode *aw_filter_;

			AutoWah(AudioContext *ctx):Effect(ctx) {
				wave_shaper_ = new WaveShaperNode(ctx);
				aw_follower_ = new BiquadNode(ctx);
				aw_follower_->frequency_->value_ = 10.0f;

				float *curve = new float[65536];
				for (auto i = -32768; i < 32768; i++) {
					curve[i + 32768] = ((i > 0)?i:-i) / 32768.0f;
				}

				wave_shaper_->SetCurve(curve, 65536);
				ctx->Connect(wave_shaper_, aw_follower_);

				aw_depth_ = new GainNode(ctx);
				aw_depth_->gain_->value_ = 11585;
				ctx->Connect(aw_follower_, aw_depth_);

				aw_filter_ = new BiquadNode(ctx);
				aw_filter_->Q_->value_ = 15;
				aw_filter_->frequency_->value_ = 50;
				ctx->Connect(aw_depth_, aw_filter_->frequency_);
				ctx->Connect(aw_filter_, wet_);

				ctx->Connect(input_, wave_shaper_);
				ctx->Connect(input_, aw_filter_);
			}

			void SetOptions(AutoWahOptions options) {
				aw_follower_->frequency_->value_ = options.aw_follower_frequency;
				aw_depth_->gain_->value_ = options.awr_depth;
				aw_filter_->Q_->value_ = options.aw_filter_Q;
			}
		};
	}
}