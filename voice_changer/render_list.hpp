#pragma once
#include <map>
#include <vector>
#include <atomic>
#include <thread>
#include "../utility/signal.hpp"
#include "voice_renderer.hpp"
using namespace std;
struct RenderData {
	string source;
	string dest;
	int* segments;
	int segment_count;
	int effect_id;
	float* options;
	int option_count;
};

enum RenderState {
	FREE,
	RENDERING,
	SUCCEEDDED,
	FAILED,
	CANCELLED
};

struct RenderMessage {
	float* status;
	int count;
};

typedef void(*OnRenderListProgress)(RenderMessage*);
typedef void(*OnRenderListEnd)(RenderMessage*);

class RenderList {
private:

	map<string, RenderData> data_;
	map<string, RenderState> state_;
	
	vector<string> list_;
	thread *render_thread_;
	mutex render_thread_mutex_;
	atomic<bool> rendering_;

	RenderMessage *progress_data_;

	int renderer_count_;
	vector<VoiceRenderer*> renderers_;

	VoiceRenderer* GetFreeRenderer() {
		for (int i = 0; i < renderer_count_; i++) {
			if (renderers_[i]->State() == VoiceRendererState::RENDERER_FREE) {
				return renderers_[i];
			}
		}
		return nullptr;
	}

	VoiceRenderer* FindRenderer(const char* s) {
		for (int i = 0; i < renderer_count_; i++) {
			if (renderers_[i]->Path() == s) {
				return renderers_[i];
			}
		}
		return nullptr;
	}

	void Join() {
		if (render_thread_ && render_thread_->joinable())
			render_thread_->join();
	}

	bool AllComplete() {
		for (auto source : list_) {
			if (state_[source] == RenderState::FREE || state_[source] == RenderState::RENDERING) {
				return false;
			}
		}
		return true;
	}

	void Traverse() {
		for (auto i = 0; i < list_.size(); i++) {
			auto data = data_[list_[i]];
			if (state_[list_[i]] != RenderState::FREE)continue;
			auto renderer = GetFreeRenderer();
			if (!renderer)break;
			auto ret = renderer->Open(data.source.c_str());
			if (ret < 0) {
				state_[data.source] = RenderState::FAILED;
				continue;
			}
			if (data.segments != nullptr && data.segment_count != 0) {
				renderer->RenderSegments(data.segments, data.segment_count);
			}
			else {
				renderer->RenderAll();
			}
			ret = renderer->SetEffect(Effects(data.effect_id));
			if (ret < 0) {
				state_[data.source] = RenderState::FAILED;
				continue;
			}
			if (data.options != nullptr && data.option_count > 0) {
				renderer->SetOptions(data.options, data.option_count);
			}
			renderer->SetDest(data.dest.c_str());
			ret = renderer->Start();
			if (ret < 0) {
				state_[data.source] = RenderState::FAILED;
				continue;
			}
			state_[data.source] = RenderState::RENDERING;
		}
	}

	void GenerateTickData() {
		{
			unique_lock<mutex> lck(render_thread_mutex_);
			auto count = list_.size();
			if (progress_data_->count != count) {
				DeleteArray(&progress_data_->status);
				AllocArray(count, &progress_data_->status);
				progress_data_->count = count;
			}
			for (int i = 0; i < count; i++) {
				auto state = state_[list_[i]];
				if (state == RenderState::RENDERING) {
					auto r = FindRenderer(list_[i].c_str());
					if (r)progress_data_->status[i] = r->Progress();
					else progress_data_->status[i] = 0.0;
				}
				else if (state == RenderState::CANCELLED) {
					progress_data_->status[i] = -2;
				}
				else if (state == RenderState::FAILED) {
					progress_data_->status[i] = -1;
				}
				else if (state == RenderState::SUCCEEDDED) {
					progress_data_->status[i] = 1.0;
				}
				else if (state == RenderState::FREE) {
					progress_data_->status[i] = 0.0;
				}
			}
		}
	}

	void DispatchTick() {
		GenerateTickData();
		on_tick_->Emit(progress_data_);
	}

	void DispatchEnd() {
		GenerateTickData();
		on_end_->Emit(progress_data_);
	}

	void RenderLoop() {
		while (true) {
			{
				unique_lock<mutex> lck(render_thread_mutex_);
				if (!rendering_)break;
				if (AllComplete())break;
				for (int i = 0; i < renderer_count_; i++) {
					auto renderer = renderers_[i];
					if (renderer->State() == VoiceRendererState::RENDERER_RENDERING && renderer->IsEnd()) {
						state_[renderer->Path()] = RenderState::SUCCEEDDED;
						renderer->Close();
					}
				}
				Traverse();
			}
			DispatchTick();
			this_thread::sleep_for(chrono::milliseconds(30));
		}
		DispatchEnd();
	}

public:

	static const int MAX_RENDERERS = 4;

	Signal<RenderMessage*> *on_tick_;
	Signal<RenderMessage*> *on_end_;

	explicit RenderList() {
		progress_data_ = new RenderMessage;
		progress_data_->status = nullptr;
		progress_data_->count = 0;
		render_thread_ = nullptr;
		rendering_ = false;
		on_tick_ = new Signal<RenderMessage*>();
		on_end_ = new Signal<RenderMessage*>();
		renderers_.reserve(MAX_RENDERERS);
		SetMaxRenderersRunTogether(2);
	}

	int SetMaxRenderersRunTogether(int max_count = 1){
		if (Rendering())return ACTION_NOT_PERMITTED;
		max_count = Clamp(0, MAX_RENDERERS, max_count);
		if (max_count > renderers_.size()) {
			for (auto i = renderers_.size(); i < max_count; i++) {
				renderers_.emplace_back(new VoiceRenderer());
			}
		}
		renderer_count_ = max_count;
		return SUCCEED;
	}

	void SubscribeProgress(OnRenderListProgress handler) {
		unique_lock<mutex> lck(render_thread_mutex_);
		on_tick_->On(handler);
	}

	void SubscribeEnd(OnRenderListEnd handler) {
		unique_lock<mutex> lck(render_thread_mutex_);
		on_end_->On(handler);
	}

	void AddRenderData(const char* source, const char* dest, int effect_id, float* options = nullptr, int option_count = 0, 
				int* segments = nullptr, int segment_count = 0) {
		unique_lock<mutex> lck(render_thread_mutex_);
		if (find(list_.begin(), list_.end(), source) != list_.end())return;
		list_.push_back(source);
		state_[source] = RenderState::FREE;
		data_[source] = {
			source,
			dest,
			segments,
			segment_count,
			effect_id,
			options,
			option_count
		};
	}

	bool Rendering() {
		unique_lock<mutex> lck(render_thread_mutex_);
		return rendering_;
	}

	void Start() {
		if (Rendering())return;
		rendering_ = true;
		render_thread_ = new thread(&RenderList::RenderLoop, this);
	}

	void Cancel(const char *s) {
		unique_lock<mutex> lck(render_thread_mutex_);
		state_[s] = RenderState::CANCELLED;
		auto renderer = FindRenderer(s);
		if (renderer) {
			renderer->Dispose();
			RemoveFile(renderer->Path().c_str());
		}
	}

	void CancelAll() {
		{
			unique_lock<mutex> lck(render_thread_mutex_);
			rendering_ = false;
			for (auto s : list_) {
				state_[s] = RenderState::CANCELLED;
			}
		}
		Join();
		for (int i = 0; i < renderer_count_; i++) {
			renderers_[i]->Dispose();
			RemoveFile(renderers_[i]->Path().c_str());
		}
	}

	void Clear() {
		CancelAll();
		list_.clear();
		state_.clear();
		data_.clear();
	}
};