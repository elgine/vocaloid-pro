#include "ane_helper.hpp"
#include "status.h"
#include "interface.hpp"

FREObject SetExtractTempPath(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	return FromInt(SetTempPath(ToString(argv[0])));
}

FREObject DisposeTemps(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(DisposeAllTempResources());
}

FREObject SetEffectPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	FromInt(SetEffectPreview(ToInt(argv[0])));
}

FREObject SetLoopPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	SetLoopPreview(ToBool(argv[0]));
}

FREObject SetSegmentsPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 2)return FromInt(MISS_PARAM);
	int *arr = ToIntArray(argv[0]);
	if (arr == nullptr)return FromInt(PARAM_NOT_MATCH);
	int count = ToInt(argv[1]);
	if (count <= 0)return FromInt(PARAM_NOT_MATCH);
	return FromInt(SetSegmentsPreview(arr, count));
}

FREObject OpenPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	return FromInt(OpenPreview(ToString(argv[0])));
}

FREObject SetEffectOptionsPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 2)return FromInt(MISS_PARAM);
	float* arr = (float*)ToDoubleArray(argv[0]);
	if (arr == nullptr)return FromInt(PARAM_NOT_MATCH);
	int count = ToInt(argv[1]);
	if (count <= 0)return FromInt(PARAM_NOT_MATCH);
	return FromInt(SetEffectOptionsPreview(arr, count));
}

FREObject StartPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(StartPreview());
}

FREObject StopPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(StopPreview());
}

FREObject Seek(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	return FromInt(Seek((int64_t)ToInt(argv[0])));
}

FREObject Render(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 8)return FromInt(MISS_PARAM);
	const char** sources = ToStringArray(argv[0]);
	if (sources == nullptr)goto param_not_match;
	const char** dests = ToStringArray(argv[1]);
	if (dests == nullptr)goto param_not_match;
	int* effects = ToIntArray(argv[2]);
	if (effects == nullptr)goto param_not_match;
	float* options = (float*)ToDoubleArray(argv[3]);
	if (options == nullptr)goto param_not_match;
	int* option_counts = ToIntArray(argv[4]);
	if (option_counts == nullptr)goto param_not_match;
	int* segments = ToIntArray(argv[5]);
	int* segment_counts = ToIntArray(argv[6]);
	int count = ToInt(argv[7]);
	if (count <= 0)goto param_not_match;
	return FromInt(Render(sources, dests, effects, options, option_counts, segments, segment_counts, count));
param_not_match:
	return FromInt(PARAM_NOT_MATCH);
}

FREObject StopRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	return FromInt(StopRender(ToString(argv[0])));
}

FREObject StopRenderAll(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(StopRenderAll());
}

FREObject CancelRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	return FromInt(CancelRender(ToString(argv[0])));
}

FREObject CancelRenderAll(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(CancelRenderAll());
}

FREObject ForceExitRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(ForceExitRender());
}

void ContextInitializer(
	void					 * extData,
	const uint8_t			 * ctxType,
	FREContext				   ctx,
	uint32_t				 * numFunctionsToSet,
	const FRENamedFunction	** functionsToSet) {
	int fn_count = 16;
	static FRENamedFunction fns[] = {
		{ (const uint8_t*) "setTempPath", NULL, &SetExtractTempPath },
		{ (const uint8_t*) "disposeTemps", NULL, &DisposeTemps },
		{ (const uint8_t*) "setEffectPreview", NULL, &SetEffectPreview },
		{ (const uint8_t*) "setLoopPreview", NULL, &SetLoopPreview },
		{ (const uint8_t*) "setSegmentsPreview", NULL, &SetSegmentsPreview },
		{ (const uint8_t*) "openPreview", NULL, &OpenPreview },
		{ (const uint8_t*) "setEffectOptionsPreview", NULL, &SetEffectOptionsPreview },
		{ (const uint8_t*) "startPreview", NULL, &StartPreview },
		{ (const uint8_t*) "stopPreview", NULL, &StopPreview },
		{ (const uint8_t*) "seek", NULL, &Seek },
		{ (const uint8_t*) "render", NULL, &Render },
		{ (const uint8_t*) "stopRender", NULL, &StopRender },
		{ (const uint8_t*) "stopRenderAll", NULL, &StopRenderAll },
		{ (const uint8_t*) "cancelRender", NULL, &CancelRender },
		{ (const uint8_t*) "cancelRenderAll", NULL, &CancelRenderAll },
		{ (const uint8_t*) "forceExitRender", NULL, &ForceExitRender }
	};
	*numFunctionsToSet = fn_count;
	*functionsToSet = fns;
}

void ContextFinalizer(FREContext ctx) {
	return;
}

extern "C" BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		SetCurrentModulePathAsDllPath();
	}
	break;
	case DLL_PROCESS_DETACH:
	{
		ResetDllPath();
	}
	break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

extern "C"
{
	__declspec(dllexport) void ExtensionInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer)
	{
		*ctxInitializer = &ContextInitializer; // The name of function that will intialize the extension context
		*ctxFinalizer = &ContextFinalizer; // The name of function that will finalize the extension context
	}

	__declspec(dllexport) void ExtensionFinalizer(void* extData)
	{
		return;
	}
}