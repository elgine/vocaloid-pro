#include "voice_player.hpp"
#include "render_list.hpp"

#define DLLEXPORT __declspec(dllexport)

static VoicePlayer *player = nullptr;
static RenderList *render_list = nullptr;

VoicePlayer* GetPlayer() {
	if (player == nullptr)player = new VoicePlayer();
	return player;
}

RenderList* GetRenderList() {
	if (render_list == nullptr)render_list = new RenderList();
	return render_list;
}

extern "C" {
	DLLEXPORT void SubscribePlayerTick(OnPlayerTick tick) {
		GetPlayer()->SubscribeTick(tick);
	}

	/*DLLEXPORT void SubscribePlayerEnd(OnPlayerEnd end) {
		GetPlayer()->SubscribeEnd(end);
	}*/

	DLLEXPORT void SubscribeRenderListProgress(OnRenderListProgress p) {
		GetRenderList()->SubscribeProgress(p);
	}

	DLLEXPORT void SubscribeRenderListComplete(OnRenderListComplete c) {
		GetRenderList()->SubscribeComplete(c);
	}

	DLLEXPORT void SubscribeRenderListEnd(OnRenderListEnd c) {
		GetRenderList()->SubscribeEnd(c);
	}

	DLLEXPORT int SetExtractTempPath(const char* path) {
		return SetTempPath(path);
	}

	DLLEXPORT int DisposeAllTemps() {
		return DisposeAllTempResources();
	}

	DLLEXPORT int Seek(int64_t timestamp) {
		return GetPlayer()->Seek(timestamp);
	}

	DLLEXPORT void SetLoopPreview(bool v) {
		GetPlayer()->Loop(v);
	}

	DLLEXPORT int SetSegmentsPreview(int* segs, int count) {
		return GetPlayer()->PlaySegments(segs, count);
	}

	DLLEXPORT int SetEffectPreview(int id) {
		return GetPlayer()->SetEffect(Effects(id));
	}

	DLLEXPORT int OpenPreview(const char* path) {
		return GetPlayer()->Open(path);
	}

	DLLEXPORT int SetEffectOptionsPreview(float* options, int count) {
		return GetPlayer()->SetOptions(options, count);
	}

	DLLEXPORT int StartPreview() {
		return GetPlayer()->Start();
	}

	DLLEXPORT int StopPreview() {
		return GetPlayer()->Stop();
	}

	DLLEXPORT int Resume() {
		return GetPlayer()->Resume();
	}

	DLLEXPORT int Render(const char** sources, const char **dests, int *effect_ids,
		float *options, int *option_counts, int* segments, int *segment_counts, int count) {
		auto option_offset = 0;
		auto segment_offset = 0;
		for (auto i = 0; i < count; i++) {
			render_list->AddRenderData(sources[i], dests[i], effect_ids[i], 
				options + option_offset, option_counts[i],
				segments + segment_offset, segment_counts[i]);
			option_offset += option_counts[i];
			segment_offset += segment_counts[i];
		}
		render_list->Start();
		return SUCCEED;
	}

	DLLEXPORT int StopRender(const char* source) {
		GetRenderList()->Stop(source);
		return SUCCEED;
	}

	DLLEXPORT int StopRenderAll() {
		GetRenderList()->StopAll();
		return SUCCEED;
	}

	DLLEXPORT int CancelRender(const char* source) {
		GetRenderList()->Cancel(source);
		return SUCCEED;
	}

	DLLEXPORT int CancelRenderAll() {
		GetRenderList()->CancelAll();
		return SUCCEED;
	}

	DLLEXPORT int ForceExitRender() {
		GetRenderList()->ForceExitRender();
		DisposeAllTempResources();
		return SUCCEED;
	}
}

