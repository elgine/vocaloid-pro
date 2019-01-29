#pragma once
#ifdef _WIN32 || _WIN64
#include "wave_out_player.hpp"
#endif
#include "player.h"
#include "destination_node.hpp"
namespace vocaloid {
	namespace node {
		class PlayerNode : public DestinationNode {	
		private:
			io::Player *player_ = nullptr;
			Buffer<char>* bytes_;
			bool inited_;
		public:
			explicit PlayerNode(BaseAudioContext *ctx) :DestinationNode(ctx) {
				output_type_ = OutputType::PLAYER;
				player_ = nullptr;
#ifdef _WIN32 || _WIN64

				player_ = new io::WaveOutPlayer();
#endif
				bytes_ = new Buffer<char>();
				sample_rate_ = 0;
				inited_ = false;
			}

			void Dispose() override {
				if (player_ == nullptr)return;
				player_->Flush();
				player_->Dispose();
				delete player_;
				player_ = nullptr;
				DestinationNode::Dispose();
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				DestinationNode::Initialize(sample_rate, frame_size); 
				int ret = 0;
				if (!inited_) {
					ret = player_->Open(sample_rate, BITS_PER_SEC, channels_);
					if (ret < 0)return ret;
					int64_t size = frame_size * channels_ * BITS_PER_SEC / 8;
					bytes_->Alloc(size);
					bytes_->SetSize(size);
					inited_ = true;
				}
				player_->Start();
				return ret;
			}

			bool Empty() {
				return player_->Empty();
			}

			int64_t Flush() override {
				return player_->Flush();
			}

			void Clear() override {
				player_->Clear();
			}

			void Stop() override {
				player_->Stop();
			}

			int64_t Processed() override {
				return player_->Played();
			}

			int64_t ProcessFrame(bool flush = false) override {
				if (player_ == nullptr || summing_buffer_->silence_)return 0;
				int64_t size = 0;
				summing_buffer_->ToByteArray(BITS_PER_SEC, bytes_->Data(), size);
				player_->Push(bytes_->Data(), size);
				return summing_buffer_->Size();
			}
		};
	}
}