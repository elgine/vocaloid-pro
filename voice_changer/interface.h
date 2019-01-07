#pragma once
#include <stdint.h>
extern "C" {

	void SetExtractTempPath(const char* path);

	void DisposeTemp();
	
	int SetPreviewEffect(int id);

	void SetPreviewLoop(bool v);

	int SetPreviewSegments(int64_t**, int count);

	int OpenPreview(const char* path);

	void SetPreviewEffectOptions(float* options, int count);

	int StartPreview();

	int StopPreview();

	void Seek(int64_t timestamp);

	void Render(const char** sources, const char **dest, int *effect_ids,
				float **options, int *option_count, int count,
				int64_t*** segmentses = nullptr, int *segment_counts = nullptr);

	void StopRender(const char* source);

	void StopRenderAll();

	void CancelRender(const char* source);

	void CancelRenderAll();

	void ForceExitRender();
}