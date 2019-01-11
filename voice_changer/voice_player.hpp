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

struct TickData {
	int64_t processed_time;
	int64_t played_time;
};
typedef void(*OnPlayerTick)(TickData);

class VoicePlayer {

private:
	Signal<TickData> *on_tick_;

	AudioContext *ctx_;
	PlayerNode *player_;
	FileReaderNode *source_reader_;
	Effect* effect_;

	thread *tick_thread_;
	mutex tick_mutex_;
	atomic<VoicePlayerState> state_;
	string path_;
	

	void Tick() {
		auto played_time = 0, processed_time = 0;
		while (1) {
			if (!Playing())break;
			ctx_->Lock();
			processed_time = FramesToMsec(source_reader_->SourceSampleRate(), source_reader_->Played());
			played_time = FramesToMsec(player_->SampleRate(), player_->Processed()) % source_reader_->Duration();
			ctx_->Unlock();
			on_tick_->Emit({processed_time, played_time});
			this_thread::sleep_for(chrono::milliseconds(22));
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
		state_ = VoicePlayerState::PLAYER_STOP;
		on_tick_ = new Signal<TickData>();
	}

	void SubscribeTick(OnPlayerTick h) {
		on_tick_->On(h);
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
		else Stop();
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
		state_ = VoicePlayerState::PLAYER_PLAYING;
		tick_thread_ = new thread(&VoicePlayer::Tick, this);
		return ret;
	}

	int Resume() {
		if (!Playing())return SUCCEED;
		source_reader_->Resume();
		if (effect_) {
			effect_->Resume();
		}
		ctx_->Start();
		{
			unique_lock<mutex> lck(tick_mutex_);
			state_ = VoicePlayerState::PLAYER_PLAYING;
		}
		tick_thread_ = new thread(&VoicePlayer::Tick, this);
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
			return ret;
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