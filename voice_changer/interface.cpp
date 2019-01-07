#include "interface.h"
#include "voice_player.hpp"
#include "render_list.hpp"

VoicePlayer *player = new VoicePlayer();
RenderList *render_list = new RenderList();

extern "C" {

	void SetExtractTempPath(const char* path) {
		SetTempPath(path);
	}

	void DisposeTemp() {
		DisposeAllTempResources();
	}

	void Seek(int64_t timestamp) {
		player->Seek(timestamp);
	}

	void SetPreviewLoop(bool v) {
		player->Loop(v);
	}

	int SetPreviewSegments(int64_t** segs, int count) {
		return player->PlaySegments(segs, count);
	}

	int SetPreviewEffect(int id) {
		return player->SetEffect(Effects(id));
	}

	int OpenPreview(const char* path) {
		return player->Open(path);
	}

	void SetPreviewEffectOptions(float* options, int count) {
		player->SetOptions(options, count);
	}

	int StartPreview() {
		return player->Start();
	}

	int StopPreview() {
		return player->Stop();
	}

	void Render(const char** sources, const char **dest, int *effect_ids,
		float **options, int *option_count, int count,
		int64_t*** segmentses, int *segment_counts) {
		auto set_segments = bool(segmentses != nullptr);
		auto set_options = bool(options != nullptr);
		for (auto i = 0; i < count; i++) {
			auto segs = set_segments ? segmentses[i] : nullptr;
			auto seg_c = set_segments ? segment_counts[i] : 0;
			render_list->AddRenderData(sources[i], dest[i], effect_ids[i], set_options?options[i]:nullptr, set_options?option_count[i]:0, segs, seg_c);
		}
		render_list->Start();
	}

	void StopRender(const char* source) {
		render_list->Stop(source);
	}

	void StopRenderAll() {
		render_list->StopAll();
	}

	void CancelRender(const char* source) {
		render_list->Cancel(source);
	}

	void CancelRenderAll() {
		render_list->CancelAll();
	}

	void ForceExitRender() {
		render_list->ForceExitRender();
	}
}