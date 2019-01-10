#pragma once
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../utility/signal.hpp"
#include "effect.hpp"
#include "factory.hpp"
#include "status.h"
#include <sstream>
using namespace std;
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
	atomic<VoicePlayerState> state_;
	thread *tick_thread_;
	mutex tick_mutex_;
	string path_;

	void OnTick() {
		int64_t last = 0, cur = 0;
		while (true) {
			{
				unique_lock<mutex> lck(tick_mutex_);
				ctx_->Lock();
				cur = player_->Processed();
				ctx_->Unlock();
				if (state_ == VoicePlayerState::PLAYER_STOP) {
					break;
				}
				last = cur;
			}
			//on_tick_->Emit(BuildTickMsg(cur));
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

	Signal<string> *on_tick_;

	explicit VoicePlayer(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::PLAYER, sample_rate, channels);
		player_ = static_cast<PlayerNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		on_tick_ = new Signal<string>();
		effect_ = nullptr;
		tick_thread_ = nullptr;
		state_ = VoicePlayerState::PLAYER_STOP;
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

	int SetOptions(float* options, int option_count) {
		int ret = SUCCEED;
		ctx_->Lock();
		if (effect_) {
			effect_->SetOptions(options, option_count);
		}
		else
			ret = NO_SUCH_EFFECT;
		ctx_->Unlock();
		return ret;
	}

	int Open(const char* path) {
		auto ret = Stop();
		if (ret < 0)return ret;
		ret = source_reader_->Open(path);
		if (ret < 0)return ret;
		path_ = path;
		return ret;
	}

	int SetEffect(Effects id) {
		if (effect_ == nullptr || (effect_ && effect_->Id() != id)) {
			auto playing = Playing();
			Stop();
			Effect* new_effect = EffectFactory(id, ctx_);
			if (new_effect == nullptr)return NO_SUCH_EFFECT;
			if (effect_ != nullptr) {
				effect_->Dispose();
				delete effect_;
				effect_ = nullptr;
			}
			effect_ = new_effect;
			if (playing) {
				source_reader_->Resume();
				Start(true); 
			}
		}
		return SUCCEED;
	}

	int PlayAll() {
		ctx_->Lock();
		auto ret = source_reader_->Start();
		ctx_->Unlock();
		return ret;
	}

	int PlaySegments(int *segments, int segment_count) {
		ctx_->Lock();
		auto ret = source_reader_->StartWithSegments(segments, segment_count);
		ctx_->Unlock();
		return ret;
	}

	bool Playing() {
		unique_lock<mutex> lck(tick_mutex_);
		return state_ == VoicePlayerState::PLAYER_PLAYING;
	}

	int Start(bool restart = true) {
		if (Playing())return SUCCEED;
		auto ret = SUCCEED;
		ctx_->stop_eof_ = false;
		if (effect_) {
			ctx_->Disconnect(source_reader_, player_);
			ctx_->Connect(source_reader_, effect_->Input());
			ctx_->Connect(effect_->Output(), player_);
		}
		else {
			ctx_->Connect(source_reader_, player_);
		}
		if (source_reader_->SegmentCount() == 0)PlayAll();
		if (effect_) {
			effect_->Start();
		}
		if (restart) {
			ret = ctx_->Prepare();
			if (ret < 0)return ret;
		}
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
		if (Playing())return SUCCEED;
		ctx_->Start();
		Join();
		{
			unique_lock<mutex> lck(tick_mutex_);
			state_ = VoicePlayerState::PLAYER_PLAYING;
		}
		tick_thread_ = new thread(&VoicePlayer::OnTick, this);
		return SUCCEED;
	}

	int Stop() {
		if (Playing()) {
			auto ret = ctx_->Stop();
			{
				unique_lock<mutex> lck(tick_mutex_);
				state_ = VoicePlayerState::PLAYER_STOP;
			}
			Join();
		}else
			return SUCCEED;
	}

	int Seek(int64_t timestamp) {
		int ret = SUCCEED;
		ctx_->Lock();
		if (source_reader_) {
			source_reader_->Seek(timestamp);
		}
		else {
			ret = HAVE_NOT_DEFINED_SOURCE;
		}
		ctx_->Unlock();
		return ret;
	}

	string Path() {
		return path_;
	}
};