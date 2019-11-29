#pragma once
#include "effect.hpp"
#include "effect_factory.hpp"
namespace effect {

	class EffectChain {
	private:
		BaseAudioContext* ctx_;
		vector<Effect*> effects_;
	public:

		explicit EffectChain(BaseAudioContext *ctx, int max_size = 100) {
			ctx_ = ctx;
			effects_.reserve(max_size);
		}

		void SetChain(Effects* ids, int effect_count) {
			DisposeEffects();
			Effect* e = nullptr;
			for (auto i = 0; i < effect_count; i++) {
				e = EffectFactory(ids[i], ctx_);
				if (e) {
					effects_.push_back(e);
					if (i > 0 && effects_[i - 1] && effects_[i]) {
						ctx_->Connect(effects_[i - 1]->Output(), effects_[i]->Input());
					}
				}
			}
		}

		void SetOptions(float *options, int* option_counts, int count) {
			int offset = 0;
			for (auto i = 0; i < count; i++) {
				if (effects_[i]) {
					effects_[i]->SetOptions(options + offset, option_counts[i]);
				}
				offset += option_counts[i];
			}
		}

		void DisposeEffects() {
			for (vector<Effect *>::iterator it = effects_.begin(); it != effects_.end(); it++) {
				if (*it != nullptr) {
					delete *it;
					*it = nullptr;
				}
			}	
			effects_.clear();
		}

		void Dispose() {
			DisposeEffects();
		}

		AudioNode* Input() {
			return effects_[0] ? effects_[0]->Input() : nullptr;
		}

		AudioNode* Output() {
			auto size = effects_.size();
			return effects_[size - 1] ? effects_[size - 1]->Output() : nullptr;
		}
	};
}