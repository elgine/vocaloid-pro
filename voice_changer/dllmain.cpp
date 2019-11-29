//#include "voice_player.hpp"
#include "render_list.hpp"

#define DLLEXPORT __declspec(dllexport)

//static VoicePlayer *player = nullptr;
static RenderList *render_list = nullptr;

//VoicePlayer* GetPlayer() {
//	if (player == nullptr)player = new VoicePlayer();
//	return player;
//}

RenderList* GetRenderList() {
	if (render_list == nullptr)render_list = new RenderList();
	return render_list;
}

extern "C" {
	/*DLLEXPORT void SubscribePlayerTick(OnPlayerTick tick) {
		GetPlayer()->SubscribeTick(tick);
	}

	DLLEXPORT void SubscribePlayerEnd(OnPlayerEnd end) {
		GetPlayer()->SubscribeEnd(end);
	}

	DLLEXPORT void SubscribePlayerStop(OnPlayerStop stop) {
		GetPlayer()->SubscribeStop(stop);
	}*/

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

	/*DLLEXPORT int Seek(int timestamp) {
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
	}*/

	DLLEXPORT int SetMaxRenderersRunTogether(int c) {
		return GetRenderList()->SetMaxRenderersRunTogether(c);
	}

	DLLEXPORT int Render(const char** sources, const char **dests, 
		int *role_ids, double *roles_options, int *role_options_counts, 
		int *env_ids, double *envs_options, int *env_options_counts,
		int *effects, double *effects_options, int *effect_options_counts, int* effect_counts,
		int* segments, int *segment_counts, double* equalizers, double* gains, int count) {
		auto role_option_offset = 0;
		auto env_option_offset = 0;
		auto segment_offset = 0;
		auto effect_id_offset = 0;
		auto effect_option_offset = 0;
		if (segments == nullptr)segment_counts = 0;
		for (auto i = 0; i < count; i++) {
			auto segment_count = segment_counts == nullptr ? 0 : segment_counts[i];
			auto role_option_count = roles_options == nullptr ? 0 : roles_options[i];
			auto env_option_count = envs_options == nullptr ? 0 : envs_options[i];
			auto equalizer = equalizers == nullptr ? nullptr : equalizers + 10 * i;
			auto gain = gains == nullptr ? 1.0 : gains[i];
			auto effect_count = effect_counts == nullptr ? 0 : effect_counts[i];
			auto effect_ids = effects == nullptr ? 0 : effects + effect_id_offset;
			auto effect_option_counts = effect_options_counts == nullptr ? 0 : effect_options_counts + effect_id_offset;
			auto effect_options_count = 0;
			for (auto j = 0; j < effect_count; j++) {
				effect_options_count += effect_options_counts == nullptr ? 0 : effect_options_counts[j + effect_id_offset];
			}
			GetRenderList()->AddRenderData(sources[i], dests[i], 
				role_ids[i], roles_options + role_option_offset, role_option_count,
				env_ids[i], envs_options + env_option_offset, env_option_count,
				effect_ids, effect_count, effects_options + effect_option_offset, effect_option_counts,
				segments + segment_offset, segment_count, equalizer, gain);
			role_option_offset += role_option_count;
			segment_offset += segment_count;
			effect_id_offset += effect_count;
			effect_option_offset += effect_options_count;
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

	DLLEXPORT void Dispose() {
		/*if (player != nullptr) {
			player->Dispose();
			delete player;
			player = nullptr;
		}*/
		if (render_list != nullptr) {
			render_list->Dispose();
			delete render_list;
			render_list = nullptr;
		}
	}
}

