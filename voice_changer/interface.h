#pragma once
#include "ane.h"

extern "C" {

	void SetExtractTempPath(const char* path);
	
	int SetEffect(int id);

	void SetLoop(bool v);

	int SetPlaySegments(int64_t**, int count);

	int Open(const char* path);

	void SetOptions(float* options, int count);

	int Play();

	int StopPlaying();
}