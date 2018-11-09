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
		public:
			explicit PlayerNode(AudioContext *ctx) :DestinationNode(ctx) {
				player_ = nullptr;
#ifdef _WIN32 || _WIN64
				player_ = new io::PCMPlayer();
#endif
			}

			void Initialize(uint32_t sample_rate, uint64_t frame_size) override {
				DestinationNode::Initialize(sample_rate, frame_size);
				if (player_ == nullptr)return;
				player_->Open(sample_rate_, BITS_PER_SEC, channels_);
			}

			void Close() override {
				if (player_ == nullptr)return;
				player_->Flush();
				player_->Close();
			}

			int64_t ProcessFrame() override {
				if (player_ == nullptr)return 0;
				uint64_t size = summing_buffer_->Size();
				uint64_t byte_len = size * summing_buffer_->Channels() * BITS_PER_SEC / 8;
				auto bytes = new char[byte_len];
				summing_buffer_->ToByteArray(BITS_PER_SEC, bytes, byte_len);
				player_->Push(bytes, byte_len);
				delete[] bytes;
				bytes = nullptr;
				return size;
			}
		};
	}
}