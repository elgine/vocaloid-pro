#pragma once
#include "composite.hpp"
#include "wave_shaper_node.hpp"
#include "biquad_node.hpp"

namespace vocaloid {
	namespace composite {
		using namespace vocaloid::node;
		class AutoWah : public Composite {
		public:

			static float ENVELOPE_FOLLOWER_FILTER_FREQUENCY_DEFAULT;
			static float ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MIN;
			static float ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MAX;

			static float FILTER_DEPTH_DEFAULT;
			static float FILTER_DEPTH_MIN;
			static float FILTER_DEPTH_MAX;

			static float FILTER_Q_DEFAULT;
			static float FILTER_Q_MIN;
			static float FILTER_Q_MAX;

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

			explicit AutoWah(BaseAudioContext *ctx):Composite(ctx) {
				wave_shaper_ = new WaveShaperNode(ctx);
				aw_follower_ = new BiquadNode(ctx);
				aw_follower_->frequency_->value_ = 10.0f;

				float *curve = new float[65536];
				for (auto i = -32768; i < 32768; i++) {
					curve[i + 32768] = ((i > 0)?i:-i) / 32768.0f;
				}
				wave_shaper_->SetCurve(curve, 65536);
				
				delete[] curve;
				curve = nullptr;

				aw_depth_ = new GainNode(ctx);
				aw_depth_->gain_->value_ = 11585;
				

				aw_filter_ = new BiquadNode(ctx);
				aw_filter_->Q_->value_ = 15;
				aw_filter_->frequency_->value_ = 50;
				

				ctx->Connect(input_, wave_shaper_);
				ctx->Connect(wave_shaper_, aw_follower_);
				ctx->Connect(aw_follower_, aw_depth_);
				ctx->Connect(aw_depth_, aw_filter_->frequency_);

				ctx->Connect(input_, aw_filter_);
				ctx->Connect(aw_filter_, wet_);
				

				SetOptions({
					10.0f,
					3.5f,
					5.0f
				});
			}

			void Dispose() override {
				if (wave_shaper_) {
					wave_shaper_->Dispose();
					delete wave_shaper_;
					wave_shaper_ = nullptr;
				}
				if (aw_follower_) {
					aw_follower_->Dispose();
					delete aw_follower_;
					aw_follower_ = nullptr;
				}
				if (aw_depth_) {
					aw_depth_->Dispose();
					delete aw_depth_;
					aw_depth_ = nullptr;
				}
				
				if (aw_filter_) {
					aw_filter_->Dispose();
					delete aw_filter_;
					aw_filter_ = nullptr;
				}
				
				Composite::Dispose();
			}

			void SetEnvelopeFollowerFilterFreq(float v) {
				aw_follower_->frequency_->value_ = v;
			}

			void SetFilterDepth(float v) {
				aw_depth_->gain_->value_ = pow(2, 10 + float(v));
			}

			void SetFilterQ(float v) {
				aw_filter_->Q_->value_ = v;
			}

			void SetOptions(AutoWahOptions options) {
				aw_follower_->frequency_->value_ = options.envelope_follower_filter_frequency;
				aw_depth_->gain_->value_ = pow(2, 10 + float(options.filter_depth));
				aw_filter_->Q_->value_ = options.filter_Q;
			}
		};

		float AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_DEFAULT = 10.0f;
		float AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MIN = 0.25f;
		float AutoWah::ENVELOPE_FOLLOWER_FILTER_FREQUENCY_MAX = 20.0f;
		float AutoWah::FILTER_DEPTH_DEFAULT = 3.5f;
		float AutoWah::FILTER_DEPTH_MIN = 0.0f;
		float AutoWah::FILTER_DEPTH_MAX = 4.0f;
		float AutoWah::FILTER_Q_DEFAULT = 5.0f;
		float AutoWah::FILTER_Q_MIN = 0.0f;
		float AutoWah::FILTER_Q_MAX = 20.0f;
	}
}