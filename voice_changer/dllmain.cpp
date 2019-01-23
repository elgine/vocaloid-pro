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

	DLLEXPORT void SubscribePlayerEnd(OnPlayerEnd end) {
		GetPlayer()->SubscribeEnd(end);
	}

	DLLEXPORT void SubscribeRenderListProgress(OnRenderListProgress p) {
		GetRenderList()->SubscribeProgress(p);
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

	DLLEXPORT int Seek(int timestamp) {
		return GetPlayer()->Seek(timestamp);
	}

	DLLEXPORT void SetLoopPreview(bool v) {
		GetPlayer()->Loop(v);
	}

	DLLEXPORT void GetSourceInfo(int* arr) {
		auto f = GetPlayer()->SourceFormat();
		arr[0] = f->sample_rate;
		arr[1] = f->bits;
		arr[2] = f->channels;
		arr[3] = f->block_align;
		arr[4] = GetPlayer()->SourceDuration();
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

	DLLEXPORT int SetEffectOptionsPreview(double* options, int count) {
		return GetPlayer()->SetOptions(options, count);
	}

	DLLEXPORT int StartPreview() {
		return GetPlayer()->Start();
	}

	DLLEXPORT int StopPreview() {
		return GetPlayer()->Stop();
	}

	DLLEXPORT int SetMaxRenderersRunTogether(int c) {
		return GetRenderList()->SetMaxRenderersRunTogether(c);
	}

	DLLEXPORT int Render(const char** sources, const char **dests, int *effect_ids,
		double *options, int *option_counts, int* segments, int *segment_counts, int count) {
		auto option_offset = 0;
		auto segment_offset = 0;
		for (auto i = 0; i < count; i++) {
			GetRenderList()->AddRenderData(sources[i], dests[i], effect_ids[i], 
				options + option_offset, option_counts[i],
				segments + segment_offset, segment_counts[i]);
			option_offset += option_counts[i];
			segment_offset += segment_counts[i];
		}
		GetRenderList()->Start();
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

	DLLEXPORT int ClearRenderList() {
		GetRenderList()->Clear();
		return SUCCEED;
	}
}

