#include "composite.hpp"
#include "wave_shaper_node.hpp"
#include "biquad_node.hpp"

namespace vocaloid {
	namespace composite {
		using namespace vocaloid::node;
		class AutoWah : public Composite {
		public:
			struct AutoWahOptions {
				// 10, [0.25, 20]
				float envelope_follower_filter_frequency;
				// 3.5, [0, 4]
				float filter_depth;
				// 5.0, [0, 20]
				float filter_Q;
			};

			WaveShaperNode *wave_shaper_;
			BiquadNode *aw_follower_;
			GainNode *aw_depth_;
			BiquadNode *aw_filter_;

			AutoWah(AudioContext *ctx):Composite(ctx) {
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

				SetOptions({
					10.0f,
					3.5f,
					5.0f
				});
			}

			void SetOptions(AutoWahOptions options) {
				aw_follower_->frequency_->value_ = options.envelope_follower_filter_frequency;
				aw_depth_->gain_->value_ = pow(2, 10 + float(options.filter_depth));
				aw_filter_->Q_->value_ = options.filter_Q;
			}

			void Dispose() override {
				wave_shaper_->Dispose();
				delete wave_shaper_;
				wave_shaper_ = nullptr;

				aw_follower_->Dispose();
				delete aw_follower_;
				aw_follower_ = nullptr;

				aw_depth_->Dispose();
				delete aw_depth_;
				aw_depth_ = nullptr;

				aw_filter_->Dispose();
				delete aw_filter_;
				aw_filter_ = nullptr;
				Composite::Dispose();
			}
		};
	}
}