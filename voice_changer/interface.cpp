#include "interface.h"
#include "voice_player.hpp"
#include "voice_renderer.hpp"

VoicePlayer *player = new VoicePlayer();
VoiceRenderer *renderer = new VoiceRenderer();

extern "C" {

	void SetExtractTempPath(const char* path) {
		SetTempPath(path);
	}

	void SetLoop(bool v) {
		player->Loop(v);
	}

	int SetPlaySegments(int64_t** segs, int count) {
		return player->PlaySegments(segs, count);
	}

	int SetEffect(int id) {
		return player->SetEffect(Effects(id));
	}

	int Open(const char* path) {
		return player->Open(path);
	}

	void SetOptions(float* options, int count) {
		player->SetOptions(options, count);
	}

	int Play() {
		return player->Start();
	}

	int StopPlaying() {
		return player->Stop();
	}
}