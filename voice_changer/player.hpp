#pragma once
#include <atomic>
#include <thread>
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "timeline.hpp"
#include "effect.hpp"
#include "effects.h"
#include "events.h"
#include "factory.hpp"
#include "status.h"
using namespace std;
using namespace vocaloid;
using namespace vocaloid::node;
using namespace effect;

enum PlayerState {
	PLAYER_PLAYING,
	PLAYER_STOPPED,
	PLAYER_DISPOSED
};

class Player{
private:
	FileReaderNode *source_;
	AudioContext *ctx_;
	Effect *effect_;
	Timeline *timeline_;
	atomic<PlayerState> state_;

	condition_variable tick_condition_;
	mutex tick_mutex_;
	thread* tick_thread_;

	void OnTick() {
		int64_t samples, timestamp;
		while (true) {
			{
				unique_lock<mutex> lck(tick_mutex_);
				if (state_ != PlayerState::PLAYER_DISPOSED)break;
				while (state_ == PlayerState::PLAYER_STOPPED)
					tick_condition_.wait(lck);
			}

			this_thread::sleep_for(chrono::milliseconds(20));
		}
	}

	void OnCtxTick(int64_t processed) {
		int64_t timestamp;
		ctx_->Lock();
		timestamp = processed / ctx_->SampleRate() * 1000;
		ctx_->Unlock();
		{
			unique_lock<mutex> lck(tick_mutex_);
			auto segment = timeline_->Seek(timestamp);
			if (segment.start < 0 || segment.end < 0) {
				auto ret = timeline_->Next();
				if (ret < 0) {
					
				}
			}
		}
	}

public:

	explicit Player(int32_t sample_rate = 44100, int16_t channels = 2){
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::PLAYER, sample_rate, channels);
		source_ = new FileReaderNode(ctx_);
		effect_ = nullptr;
		tick_thread_ = nullptr;
		timeline_ = new Timeline();
		ctx_->on_tick_->On(bind(&Player::OnCtxTick, this));
	}

	void SetFormat(int32_t sample_rate, int16_t channels) {
		ctx_->Destination()->SetFormat(sample_rate, channels);
	}

	int SetSegments(int64_t** segments, int segment_count) {
		if (segment_count > 0) {
			if (sizeof(segments[0]) / sizeof(int64_t) < 2) {
				return INVALIDATED_SEGEMENT_DATA;
			}
		}
		else {
			return INVALIDATED_SEGEMENT_DATA;
		}
		timeline_->Set(segments, segment_count);
		return SUCCEED;
	}

	int SetSource(const char* source) {
		return source_->SetPath(source);
	}

	int SetEffect(int effect_id) {
		if (effect_ != nullptr) {
			effect_->Dispose();
			delete effect_;
			effect_ = nullptr;
		}
		effect_ = EffectFactory(Effects(effect_id), ctx_);
		if (effect_ == nullptr) {
			return NO_SUCH_EFFECT;
		}
	}

	void Play(bool restart) {
		if (restart) {
			Stop();
			timeline_->Reset();
			ctx_->Destination()->Clear();
			if (effect_ != nullptr) {
				ctx_->Connect(source_, effect_->Input());
				ctx_->Connect(effect_->Output(), ctx_->Destination());
				effect_->Start();
			}
			else {
				ctx_->Connect(source_, ctx_->Destination());
			}
			source_->Start(0);
			ctx_->Prepare();
		}
		else {
			if (effect_ != nullptr)
				effect_->Start();
		}
		ctx_->Start();
		state_ = PlayerState::PLAYER_PLAYING;
		tick_condition_.notify_one();
		if(tick_thread_ == nullptr)tick_thread_ = new thread(&Player::OnTick, this);
	}

	int Seek(int64_t timestamp) {
		auto segment = timeline_->Seek(timestamp);
		if (segment.start < 0 || segment.end < 0) {
			return NOT_IN_SEGMENTS;
		}
		ctx_->Clear();
		ctx_->Lock();
		source_->Seek(timestamp);
		ctx_->Unlock();
	}

	void Stop() {
		state_ = PlayerState::PLAYER_STOPPED;
		ctx_->Stop();
	}

	void Dispose() {
		state_ = PlayerState::PLAYER_DISPOSED;
		tick_condition_.notify_one();
		if (tick_thread_ != nullptr && tick_thread_->joinable()) {
			tick_thread_->join();
			delete tick_thread_;
			tick_thread_ = nullptr;
		}
	}
};