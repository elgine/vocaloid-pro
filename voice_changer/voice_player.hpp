#pragma once
#include "../utility/logger.hpp"
#include "../vocaloid/audio_context.hpp"
#include "../vocaloid/file_reader_node.hpp"
#include "../vocaloid/player_node.hpp"
#include "../utility/signal.hpp"
#include "role.hpp"
#include "factory.hpp"
#include "status.h"
#include <sstream>
using namespace std;
using namespace vocaloid;
using namespace vocaloid::node;
using namespace role;

enum VoicePlayerState {
	PLAYER_PLAYING,
	PLAYER_STOP
};

struct PlayerTickData {
	int timestamp;
	int duration;
};

typedef void(*OnPlayerTick)(PlayerTickData);
typedef void(*OnPlayerEnd)(int);
typedef void(*OnPlayerStop)(int);

class VoicePlayer {

private:
	Signal<PlayerTickData> *on_tick_;
	Signal<int> *on_end_;
	Signal<int> *on_stop_;

	AudioContext *ctx_;
	PlayerNode *player_;
	FileReaderNode *source_reader_;
	Effect* effect_;

	string path_;

	atomic<int64_t> timestamp_;
	atomic<int64_t> start_timestamp_;
	atomic<int64_t> end_timestamp_;
	atomic<int64_t> duration_;
	int64_t last_processed_player_;

	atomic<bool> loop_;

	thread *tick_thread_;
	mutex tick_mutex_;
	atomic<VoicePlayerState> state_;

	bool inited_;

	int64_t CalcCorrectPlayTime(int64_t delta) {
		int64_t timestamp = 0;
		auto source_sample_rate = source_reader_->SourceSampleRate();
		auto resample_ratio = float(source_sample_rate) / ctx_->SampleRate();
		// 当前播放片段
		auto seg_index = source_reader_->Index();
		auto seg = source_reader_->Segment(seg_index);
		// 期望的播放位置
		auto pos = source_reader_->Played();
		// 递归，若当前播放片段的开始偏移大于期望，则
		// 证明真实的播放位置应该在当前片段之前...
		while (delta > 0 && seg.start > pos - delta) {
			// 去掉当前片段相应的偏移量
			delta -= pos - seg.start;
			if (seg_index - 1 < 0) {
				seg_index += source_reader_->SegmentCount();
			}
			seg = source_reader_->Segment(--seg_index);
			pos = seg.end;
		}
		pos -= delta;
		timestamp = FramesToMsec(source_sample_rate, pos);
		return timestamp;
	}

	void Tick() {
		int last_timestamp = 0;
		while (1) {
			if (!Playing())break;
			if (state_ == VoicePlayerState::PLAYER_STOP)break;
			if (last_timestamp != timestamp_) {
				on_tick_->Emit({ int(timestamp_), int(duration_) });
				last_timestamp = timestamp_;
			}
			if (End()) {
				if (!loop_) {
					on_end_->Emit(0);
					break;
				}
				Seek(start_timestamp_);
			}
			this_thread::sleep_for(chrono::milliseconds(33));
		}
		ctx_->Stop();
		{
			unique_lock<mutex> lck(tick_mutex_);
			state_ = VoicePlayerState::PLAYER_STOP;
		}
		on_stop_->Emit(0);
	}

	void Join() {
		if (tick_thread_ != nullptr && tick_thread_->joinable()) {
			tick_thread_->join();
			delete tick_thread_;
			tick_thread_ = nullptr;
		}
	}

	bool End() {
		unique_lock<mutex> ctx_lck(ctx_->audio_render_thread_mutex_);
		return timestamp_ >= end_timestamp_ || (source_reader_->Eof() && player_->Empty());
	}

	void UpdateTimestamp() {
		bool need_update = true;
		auto processed_player = 0;
		auto processed = 0;
		int64_t timestamp = timestamp_;
		{
			unique_lock<mutex> ctx_lck(ctx_->audio_render_thread_mutex_);
			processed = source_reader_->Processed();
			processed_player = player_->Processed();
			if (effect_) {
				processed_player = (processed_player - MsecToFrames(ctx_->SampleRate(), effect_->Delay() * 1000)) / effect_->TimeScale();
			}
			if (processed_player < 0) {
				need_update = false;
				processed_player = 0;
			}
			if (need_update) {
				timestamp = CalcCorrectPlayTime(processed - processed_player);
			}
		}
		{
			unique_lock<mutex> tick_lck(tick_mutex_);
			timestamp_ = timestamp;
		}
	}

public:

	explicit VoicePlayer(int32_t sample_rate = 44100, int16_t channels = 2) {
		ctx_ = new AudioContext();
		ctx_->SetOutput(OutputType::PLAYER, sample_rate, channels);
		ctx_->stop_eof_ = true;
		player_ = static_cast<PlayerNode*>(ctx_->Destination());
		source_reader_ = new FileReaderNode(ctx_);
		tick_thread_ = nullptr;
		state_ = VoicePlayerState::PLAYER_STOP;
		effect_ = nullptr;
		on_tick_ = new Signal<PlayerTickData>();
		on_end_ = new Signal<int>();
		on_stop_ = new Signal<int>();
		timestamp_ = 0;
		end_timestamp_ = 0;
		duration_ = 0;
		inited_ = false;
		loop_ = false;
		ctx_->on_tick_->On([&](int) {
			UpdateTimestamp();
		});

		logger::LogStart("C:\\Users\\Elgine\\Desktop\\log.txt");
	}

	void SubscribeStop(OnPlayerStop h) {
		on_stop_->On(h);
	}

	void SubscribeTick(OnPlayerTick h) {
		on_tick_->On(h);
	}

	void SubscribeEnd(OnPlayerEnd h) {
		on_end_->On(h);
	}

	void Loop(bool v) {
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			loop_ = v;
		}
	}

	void SetFormat(int32_t sample_rate = 44100, int16_t channels = 2) {
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			player_->SetFormat(sample_rate, channels);
		}
	}

	int SetOptions(double* options, int option_count) {
		static float* options_float = new float[500];
		for (auto i = 0; i < option_count; i++) {
			options_float[i] = options[i];
		}
		int ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			if (effect_) {
				effect_->SetOptions(options_float, option_count);
			}
			else
				ret = NO_SUCH_EFFECT;
		}
		Seek(timestamp_);
		return ret;
	}

	int Open(const char* path) {
		auto ret = Stop();
		if (ret < 0)return ret;
		ret = source_reader_->Open(path);
		if (ret < 0)return ret;
		path_ = path;
		PlayAll();
		/*source_reader_->Clear();
		player_->Clear();*/
		ctx_->Clear();
		timestamp_ = 0;
		duration_ = source_reader_->Duration();
		start_timestamp_ = FramesToMsec(source_reader_->SourceSampleRate(), source_reader_->FirstSegment().start);
		end_timestamp_ = FramesToMsec(source_reader_->SourceSampleRate(), source_reader_->LastSegment().end);
		inited_ = false;
		return ret;
	}

	int SourceDuration() {
		return source_reader_->Duration();
	}

	io::AudioFormat* SourceFormat() {
		return source_reader_->Format();
	}

	int SetEffect(Effects id) {
		int64_t timestamp = timestamp_;;
		auto playing = Playing();
		if (effect_ == nullptr || (effect_ && effect_->Id() != id)) {
			Stop();
			Effect* new_effect = EffectFactory(id, ctx_);
			if (effect_ != nullptr) {
				effect_->Dispose();
				delete effect_;
				effect_ = nullptr;
			}
			effect_ = new_effect;
			inited_ = false;
			Seek(timestamp);
			if (playing) {
				Start(); 
			}
		}
		return SUCCEED;
	}

	int PlayAll() {
		auto ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->Start();
		}
		return ret;
	}

	int PlaySegments(int *segments, int segment_count) {
		auto ret = SUCCEED;
		{
			unique_lock<mutex> lck(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->StartWithSegments(segments, segment_count);
		}
		return ret;
	}

	bool Playing() {
		return state_ == VoicePlayerState::PLAYER_PLAYING;
	}

	int Start() {
		if (Playing())return SUCCEED;
		Stop();
		auto ret = SUCCEED;
		source_reader_->Resume();
		if (effect_) {
			effect_->Start();
		}
		if (!inited_) {
			if (effect_) {
				ctx_->Disconnect(source_reader_, player_);
				ctx_->Connect(source_reader_, effect_->Input());
				ctx_->Connect(effect_->Output(), player_);
			}
			else {
				ctx_->Connect(source_reader_, player_);
			}
			ret = ctx_->Prepare();
			if (ret < 0)return ret;
			inited_ = true;
		}
		ctx_->Start();
		state_ = VoicePlayerState::PLAYER_PLAYING;
		tick_thread_ = new thread(&VoicePlayer::Tick, this);
		return ret;
	}

	int Stop() {
		if (Playing()) {
			{
				unique_lock<mutex> lck(tick_mutex_);
				state_ = VoicePlayerState::PLAYER_STOP;
			}
			Join();
			return SUCCEED;
		}else
			return SUCCEED;
	}

	int Seek(int64_t timestamp) {
		int ret = SUCCEED;
		unique_lock<mutex> lck(tick_mutex_);
		ctx_->Clear();
		{
			unique_lock<mutex> render(ctx_->audio_render_thread_mutex_);
			ret = source_reader_->Seek(timestamp);
			source_reader_->Resume();
		}
		timestamp_ = timestamp;
		return ret;
	}

	void Dispose() {
		Stop();
		if (on_tick_ != nullptr) {
			on_tick_->Clear();
			delete on_tick_;
			on_tick_ = nullptr;
		}
		if (on_end_ != nullptr) {
			on_end_->Clear();
			delete on_end_;
			on_end_ = nullptr;
		}
		if (on_stop_ != nullptr) {
			on_stop_->Clear();
			delete on_stop_;
			on_stop_ = nullptr;
		}
		if (source_reader_ != nullptr) {
			source_reader_->Dispose();
			delete source_reader_;
			source_reader_ = nullptr;
		}
		if (player_ != nullptr) {
			player_->Dispose();
			delete player_;
			player_ = nullptr;
		}
		if (effect_ != nullptr) {
			effect_->Dispose();
			delete effect_;
			effect_ = nullptr;
		}
		if (ctx_ != nullptr) {
			ctx_->Dispose();
			delete ctx_;
			ctx_ = nullptr;
		}
	}

	string Path() {
		return path_;
	}
};