#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ane_helper.hpp"
#include "../utility/path.hpp"
#include "../utility/string_wstring.hpp"

#define MISS_PARAM -1026
#define PARAM_NOT_MATCH -1028
#define NO_SUCH_PATH -1029
#define LOAD_LIBRARY_FAILED -1030
#define HAS_NOT_INITED -1031

const int dependency_count = 9;
const char *dependencies[dependency_count] = {
	"avutil-55.dll",
	"swresample-2.dll",
	"avcodec-57.dll",
	"avformat-57.dll",
	"postproc-54.dll",
	"swscale-4.dll",
	"avfilter-6.dll",
	"avdevice-57.dll",
	"VoiceChanger.dll"
};

bool inited = false;
int(*set_temp_path)(const char*);
int(*dispose_temps)();
int(*set_effect)(int);
int(*set_effect_options)(float*, int);
int(*set_segments)(int*, int);
void(*set_loop)(bool);
int(*open_preview)(const char*);
int(*start_preview)();
int(*stop_preview)();
int(*seek)(int);
int(*render)(const char**, const char**, int*,float*, int*, int*, int*, int);
int(*stop_render)(const char*);
int(*stop_render_all)();
int(*cancel_render)(const char*);
int(*cancel_render_all)();
int(*force_render_exit)();

FREObject Initialize(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	const char* path = ToString(argv[0]);
	if (path == nullptr)return FromInt(PARAM_NOT_MATCH);
	if (!IsPathDirectory(path))return FromInt(NO_SUCH_PATH);
	SetDllDirectory(StringToWString(path));
	HMODULE handler = nullptr;
	for (auto i = 0; i < dependency_count; i++) {
		wchar_t* dll_name = StringToWString(dependencies[i]);
		if (GetModuleHandle(dll_name) != nullptr)continue;
		handler = LoadLibrary(dll_name);
		if (handler == nullptr) {
			return FromInt(LOAD_LIBRARY_FAILED);
		}
	}
	handler = GetModuleHandle(L"VoiceChanger.dll");
	if (handler != nullptr) {
		set_temp_path = (int(*)(const char*))GetProcAddress(handler, "SetExtractTempPath");
		if(set_temp_path == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		dispose_temps = (int(*)(void))GetProcAddress(handler, "DisposeAllTemps");
		if(dispose_temps == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		seek = (int(*)(int))GetProcAddress(handler, "Seek");
		if(seek == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		set_loop = (void(*)(bool))GetProcAddress(handler, "SetLoopPreview");
		if (set_loop == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		set_segments = (int(*)(int*, int))GetProcAddress(handler, "SetSegmentsPreview");
		if(set_segments == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		set_effect = (int(*)(int))GetProcAddress(handler, "SetEffectPreview");
		if (set_effect == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		open_preview = (int(*)(const char*))GetProcAddress(handler, "OpenPreview");
		if (open_preview == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		set_effect_options = (int(*)(float*, int))GetProcAddress(handler, "SetEffectOptionsPreview");
		if (set_effect_options == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		start_preview = (int(*)())GetProcAddress(handler, "StartPreview");
		if (start_preview == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		stop_preview = (int(*)())GetProcAddress(handler, "StopPreview");
		if (stop_preview == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		render = (int(*)(const char**, const char**, int*, float*, int*, int*, int*, int))GetProcAddress(handler, "Render");
		if (render == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		stop_render = (int(*)(const char*))GetProcAddress(handler, "StopRender");
		if (stop_render == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		stop_render_all = (int(*)())GetProcAddress(handler, "StopRenderAll");
		if (stop_render_all == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		cancel_render = (int(*)(const char*))GetProcAddress(handler, "CancelRender");
		if(cancel_render == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		cancel_render_all = (int(*)())GetProcAddress(handler, "CancelRenderAll");
		if (cancel_render_all == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		force_render_exit = (int(*)())GetProcAddress(handler, "ForceExitRender");
		if (force_render_exit == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
		inited = true;
	}
	return FromInt(0);
}

FREObject SetExtractTempPath(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	return FromInt(set_temp_path(ToString(argv[0])));
}

FREObject DisposeTemps(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	return FromInt(dispose_temps());
}

FREObject SetEffectPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	FromInt(set_effect(ToInt(argv[0])));
}

FREObject SetLoopPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	set_loop(ToBool(argv[0]));
}

FREObject SetSegmentsPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 2)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	int *arr = ToIntArray(argv[0]);
	if (arr == nullptr)return FromInt(PARAM_NOT_MATCH);
	int count = ToInt(argv[1]);
	if (count <= 0)return FromInt(PARAM_NOT_MATCH);
	return FromInt(set_segments(arr, count));
}

FREObject OpenPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(open_preview(ToString(argv[0])));
}

FREObject SetEffectOptionsPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 2)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	float* arr = (float*)ToDoubleArray(argv[0]);
	if (arr == nullptr)return FromInt(PARAM_NOT_MATCH);
	int count = ToInt(argv[1]);
	if (count <= 0)return FromInt(PARAM_NOT_MATCH);
	return FromInt(set_effect_options(arr, count));
}

FREObject StartPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(start_preview());
}

FREObject StopPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(stop_preview());
}

FREObject Seek(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(seek((int64_t)ToInt(argv[0])));
}

FREObject Render(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 8)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
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
	return FromInt(render(sources, dests, effects, options, option_counts, segments, segment_counts, count));
param_not_match:
	return FromInt(PARAM_NOT_MATCH);
}

FREObject StopRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(stop_render(ToString(argv[0])));
}

FREObject StopRenderAll(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(stop_render_all());
}

FREObject CancelRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(cancel_render(ToString(argv[0])));
}

FREObject CancelRenderAll(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(cancel_render_all());
}

FREObject ForceExitRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(force_render_exit());
}

void ContextInitializer(
	void					 * extData,
	const uint8_t			 * ctxType,
	FREContext				   ctx,
	uint32_t				 * numFunctionsToSet,
	const FRENamedFunction	** functionsToSet) {
	int fn_count = 17;
	static FRENamedFunction fns[] = {
		{ (const uint8_t*) "initialize", NULL, &Initialize },
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