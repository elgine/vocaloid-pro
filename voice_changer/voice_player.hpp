#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "effect.hpp"
#include "factory.hpp"
#include "status.h"
using namespace vocaloid;
using namespace vocaloid::node;
using namespace effect;

enum VoicePlayerState {
	PLAYER_PLAYING,
	PLAYER_STOP
};

class VoicePlayer {

private:
	AudioContext *ctx_;
	PlayerNode *player_;
	FileReaderNode *source_reader_;
	Effect* effect_;
	bool inited_;
	atomic<VoicePlayerState> state_;
	thread *tick_thread_;
	mutex tick_mutex_;

	void OnTick() {
		int64_t last = 0, cur = 0;
		while (true) {
			//{
			//	unique_lock<mutex> lck(tick_mutex_);
			//	cur = player_->Processed();
			//	if (state_ == VoicePlayerState::PLAYER_STOP && cur == last) {
			//		break;
			//	}
			//	// Emit client to update timestamp
			//	last = cur;
			//}
			this_thread::sleep_for(chrono::milliseconds(20));
		}
	}

	void Join() {
		if (tick_thread_ != nullptr && tick_thread_->joinable()) {
			tick_thread_->join();
			delete tick_thread_;
			tick_thread_ = nullptr;
		}
	}

public:
	explicit VoicePlayer(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::PLAYER, sample_rate, channels);
		player_ = static_cast<PlayerNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		effect_ = nullptr;
		tick_thread_ = nullptr;
		inited_ = false;
	}

	void Loop(bool v) {
		ctx_->Lock();
		source_reader_->loop_ = v;
		ctx_->Unlock();
	}

	void SetFormat(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_->Lock();
		player_->SetFormat(sample_rate, channels);
		ctx_->Unlock();
	}

	void SetOptions(float* options, int option_count) {
		ctx_->Lock();
		if (effect_) {
			effect_->SetOptions(options, option_count);
		}
		ctx_->Unlock();
	}

	int Open(const char* path) {
		auto ret = Stop();
		if (ret < 0)return ret;
		return source_reader_->Open(path);
	}

	int SetEffect(Effects id) {
		if (effect_ == nullptr || (effect_ && effect_->Id() != id)) {
			Effect* new_effect = EffectFactory(id, ctx_);
			if (new_effect == nullptr)return NO_SUCH_EFFECT;
			if (effect_ != nullptr) {
				effect_->Dispose();
				delete effect_;
				effect_ = nullptr;
			}
			effect_ = new_effect;
		}
		return SUCCEED;
	}

	int PlayAll() {
		ctx_->Lock();
		auto ret = source_reader_->Start();
		ctx_->Unlock();
		return ret;
	}

	int PlaySegments(int64_t **segments, int segment_count) {
		ctx_->Lock();
		auto ret = source_reader_->StartWithSegments(segments, segment_count);
		ctx_->Unlock();
		return ret;
	}

	int Start(bool restart = false) {
		{
			unique_lock<mutex> lck(tick_mutex_);
			if (state_ == VoicePlayerState::PLAYER_PLAYING)return HAVE_INITED;
		}
		auto ret = SUCCEED;
		if (!inited_) {
			if (effect_) {
				ctx_->Connect(source_reader_, effect_->Input());
				ctx_->Connect(effect_->Output(), player_);
			}
			else {
				ctx_->Connect(source_reader_, player_);
			}
		}
		if (effect_) {
			effect_->Start();
		}
		if (!inited_ || (inited_ && restart)) {
			ret = ctx_->Prepare();
			if (ret < 0)return ret;
		}
		inited_ = true;
		ctx_->Start();
		Join();
		{
			unique_lock<mutex> lck(tick_mutex_);
			state_ = VoicePlayerState::PLAYER_PLAYING;
		}
		tick_thread_ = new thread(&VoicePlayer::OnTick, this);
		return ret;
	}

	int Resume() {
		auto playing = false;
		{
			unique_lock<mutex> lck(tick_mutex_);
			playing = state_ == VoicePlayerState::PLAYER_PLAYING;
		}
		if (playing)return SUCCEED; 
		if (inited_) {
			ctx_->Start();
			Join();
			{
				unique_lock<mutex> lck(tick_mutex_);
				state_ = VoicePlayerState::PLAYER_PLAYING;
			}
			tick_thread_ = new thread(&VoicePlayer::OnTick, this);
			return SUCCEED;
		}
		return HAVE_NOT_STARTED;
	}

	int Stop() {
		auto playing = false;
		{
			unique_lock<mutex> lck(tick_mutex_);
			playing = state_ == VoicePlayerState::PLAYER_PLAYING;
		}
		if (playing) {
			auto ret = ctx_->Stop();
			{
				unique_lock<mutex> lck(tick_mutex_);
				state_ = VoicePlayerState::PLAYER_STOP;
			}
			Join();
		}else
			return SUCCEED;
	}
};