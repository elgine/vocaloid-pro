#pragma once
#include "voice_player.hpp"
#include "render_list.hpp"

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

void SetExtractTempPath(const char* path) {
	SetTempPath(path);
}

void DisposeTemp() {
	DisposeAllTempResources();
}

int Seek(int64_t timestamp) {
	return GetPlayer()->Seek(timestamp);
}

void SetLoopPreview(bool v) {
	GetPlayer()->Loop(v);
}

int SetSegmentsPreview(int* segs, int count) {
	return GetPlayer()->PlaySegments(segs, count);
}

int SetEffectPreview(int id) {
	return GetPlayer()->SetEffect(Effects(id));
}

int OpenPreview(const char* path) {
	return GetPlayer()->Open(path);
}

int SetEffectOptionsPreview(float* options, int count) {
	return GetPlayer()->SetOptions(options, count);
}

int StartPreview() {
	return GetPlayer()->Start();
}

int StopPreview() {
	return GetPlayer()->Stop();
}

int Render(const char** sources, const char **dest, int *effect_ids,
	float *options, int *option_count, int* segments, int *segment_counts, int count) {
	/*auto set_segments = bool(segmentses != nullptr);
	auto set_options = bool(options != nullptr);
	for (auto i = 0; i < count; i++) {
		auto segs = set_segments ? segmentses[i] : nullptr;
		auto seg_c = set_segments ? segment_counts[i] : 0;
		render_list->AddRenderData(sources[i], dest[i], effect_ids[i], set_options ? options[i] : nullptr, set_options ? option_count[i] : 0, segs, seg_c);
	}*/
	//render_list->Start();
	return SUCCEED;
}

int StopRender(const char* source) {
	GetRenderList()->Stop(source);
	return SUCCEED;
}

int StopRenderAll() {
	GetRenderList()->StopAll();
	return SUCCEED;
}

int CancelRender(const char* source) {
	GetRenderList()->Cancel(source);
	return SUCCEED;
}

int CancelRenderAll() {
	GetRenderList()->CancelAll();
	return SUCCEED;
}

int ForceExitRender() {
	GetRenderList()->ForceExitRender();
	return SUCCEED;
}