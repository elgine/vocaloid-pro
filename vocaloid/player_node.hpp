#pragma once
#ifdef _WIN32 || _WIN64
#include "pcm_player.hpp"
#endif
#include "player.h"
#include "destination_node.hpp"
namespace vocaloid {
	namespace node {
		class PlayerNode : public DestinationNode {	
		private:
			io::Player *player_ = nullptr;
			Buffer<char>* bytes_;

		public:
			explicit PlayerNode(BaseAudioContext *ctx) :DestinationNode(ctx) {
				player_ = nullptr;
#ifdef _WIN32 || _WIN64
				player_ = new io::PCMPlayer();
#endif
				bytes_ = new Buffer<char>();
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				DestinationNode::Initialize(sample_rate, frame_size);
				if (player_ == nullptr)return UNKNOWN_EXCEPTION;
				int ret = player_->Open(sample_rate_, BITS_PER_SEC, channels_) < 0;
				if (ret < 0)return ret;
				int64_t size = frame_size * channels_ * BITS_PER_SEC / 8;
				bytes_->Alloc(size);
				bytes_->SetSize(size);
				return ret;
			}

			int64_t Processed() override {
				return player_->Played();
			}

			void Dispose() override {
				if (player_ == nullptr)return;
				player_->Flush();
				player_->Dispose();
			}

			int64_t ProcessFrame() override {
				if (player_ == nullptr || summing_buffer_->silence_)return 0;
				int64_t size = 0;
				summing_buffer_->ToByteArray(BITS_PER_SEC, bytes_->Data(), size);
				player_->Push(bytes_->Data(), size);
				return size;
			}
		};
	}
}