#pragma once
#include <map>
#include <vector>
#include <atomic>
#include <thread>
#include "../utility/signal.hpp"
#include "voice_renderer.hpp"
using namespace std;
struct RenderData {
	const char* source;
	const char* dest;
	int* segments;
	int segment_count;
	int effect_id;
	float* options;
	int option_count;
};

typedef void(*OnRenderListProgress)(float*);
typedef void(*OnRenderListEnd)(int*);
typedef void(*OnRenderListComplete)(char*);

class RenderList {
private:

	int index_;
	vector<RenderData> list_;
	vector<VoiceRenderer*> renderers_;
	vector<string> finished_;
	vector<string> failed_;
	thread *render_thread_;
	mutex render_thread_mutex_;

	atomic<bool> rendering_;

	VoiceRenderer* GetFreeRenderer() {
		for (auto renderer : renderers_) {
			if (renderer->State() == VoiceRendererState::RENDERER_FREE) {
				return renderer;
			}
		}
		return nullptr;
	}

	VoiceRenderer* FindRenderer(const char* s) {
		for (auto renderer : renderers_) {
			if (renderer->Path() == s) {
				return renderer;
			}
		}
		return nullptr;
	}

	void Join() {
		if (render_thread_ && render_thread_->joinable())
			render_thread_->join();
	}

	void RenderLoop() {
		while (true) {
			{
				unique_lock<mutex> lck(render_thread_mutex_);
				if (!rendering_)break;
				if (finished_.size() + failed_.size() == list_.size()) {
					// TODO: Complete
					break;
				}

				for (auto renderer : renderers_) {
					if(renderer->State() == VoiceRendererState::RENDERER_RENDERING && renderer->IsEnd()){
						finished_.push_back(renderer->Path());
						renderer->Close();
					}
				}
				
				for (auto i = index_; i < list_.size(); i++) {
					auto renderer = GetFreeRenderer();
					if (!renderer)break;
					index_++;
					auto data = list_[i];
					auto ret = renderer->Open(data.source);
					if (ret < 0) {
						failed_.push_back(data.source);
						continue;
					}
					if (data.segments != nullptr && data.segment_count != 0) {
						renderer->RenderSegments(data.segments, data.segment_count);
					}
					ret = renderer->SetEffect(Effects(data.effect_id));
					if (ret < 0) {
						failed_.push_back(data.source);
						continue;
					}
					renderer->SetOptions(data.options, data.option_count);
					renderer->SetDest(data.dest);
					ret = renderer->Start();
					if (ret < 0) {
						failed_.push_back(data.source);
						continue;
					}
				}
			}
			this_thread::sleep_for(chrono::milliseconds(30));
		}
	}

public:

	Signal<float*> *on_tick_;
	Signal<int*> *on_end_;
	Signal<char*> *on_complete_;

	explicit RenderList(int max_count = 1) {
		render_thread_ = nullptr;
		renderers_.reserve(max_count);
		index_ = 0;
		on_tick_ = new Signal<float*>();
		on_end_ = new Signal<int*>();
		on_complete_ = new Signal<char*>();
		for (auto i = 0; i < max_count; i++) {
			renderers_.emplace_back(new VoiceRenderer());
		}
	}

	void SubscribeComplete(OnRenderListComplete handler) {
		unique_lock<mutex> lck(render_thread_mutex_);
		on_complete_->On(handler);
	}

	void SubscribeProgress(OnRenderListProgress handler) {
		unique_lock<mutex> lck(render_thread_mutex_);
		on_tick_->On(handler);
	}

	void SubscribeEnd(OnRenderListEnd handler) {
		unique_lock<mutex> lck(render_thread_mutex_);
		on_end_->On(handler);
	}

	void AddRenderData(const char* source, const char* dest, int effect_id, float* options, int option_count, 
				int* segments = nullptr, int segment_count = 0) {
		unique_lock<mutex> lck(render_thread_mutex_);
		list_.push_back({
			source,
			dest,
			segments,
			segment_count,
			effect_id,
			options,
			option_count
		});
	}

	void Clear() {
		index_ = 0;
	}

	void ForceExitRender() {
		for (auto renderer : renderers_) {
			renderer->Dispose();
		}
		{
			unique_lock<mutex> lck(render_thread_mutex_);
			if (!rendering_)return;
			rendering_ = false;
		}
		Join();
	}

	void Start() {
		StopAll();
		rendering_ = true;
		render_thread_ = new thread(&RenderList::RenderLoop, this);
	}

	void Cancel(const char *s) {
		unique_lock<mutex> lck(render_thread_mutex_);
		auto renderer = FindRenderer(s);
		if (renderer) {
			renderer->Dispose();
			RemoveFile(renderer->Path().c_str());
		}
	}

	void CancelAll() {
		{
			unique_lock<mutex> lck(render_thread_mutex_);
			if (!rendering_)return;
			rendering_ = false;
		}
		Join();
		for (auto renderer : renderers_) {
			renderer->Dispose();
			RemoveFile(renderer->Path().c_str());
		}
	}

	void Stop(const char* s) {
		unique_lock<mutex> lck(render_thread_mutex_);
		auto renderer = FindRenderer(s);
		if(renderer)renderer->Stop();
	}

	void StopAll() {
		{
			unique_lock<mutex> lck(render_thread_mutex_);
			if (!rendering_)return;
			rendering_ = false;
		}
		Join();
		for (auto renderer : renderers_) {
			renderer->Stop();
		}
	}

	bool Rendering() {
		unique_lock<mutex> lck(render_thread_mutex_);
		return rendering_;
	}
};