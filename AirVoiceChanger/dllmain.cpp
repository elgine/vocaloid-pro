#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <sstream>
#include "ane_helper.hpp"
#include "../utility/path.hpp"
#include "../utility/string_wstring.hpp"
#include "../utility/utf8_string.hpp"
#define MISS_PARAM -1026
#define PARAM_NOT_MATCH -1028
#define NO_SUCH_PATH -1029
#define LOAD_LIBRARY_FAILED -1030
#define HAS_NOT_INITED -1031

FREContext fre_context = nullptr;
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

struct PlayerTickData {
	int timestamp;
	int duration;
};

bool inited = false;
int(*set_temp_path)(const char*);
void(*get_source_info)(int*);
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
int(*cancel_render)(const char*);
int(*cancel_render_all)();
int(*set_max_renderers_run_together)(int);
int(*clear_render_list)();

struct RenderMessage {
	float* status;
	int count;
};

typedef void(*OnRenderListProgress)(RenderMessage*);
typedef void(*OnRenderListEnd)(RenderMessage*);

typedef void(*OnPlayerTick)(PlayerTickData);
typedef void(*OnPlayerEnd)(int);

void(*subscribe_player_tick)(OnPlayerTick);
void(*subscribe_player_end)(OnPlayerEnd);

void(*subscribe_render_list_progress)(OnRenderListProgress);
void(*subscribe_render_list_end)(OnRenderListEnd);

#define PLAYER_TICK_CODE "playerTick"
#define PLAYER_END_CODE "playerEnd"
#define RENDER_LIST_PROGRESS_CODE "renderListProgress"
#define RENDER_LIST_COMPLETE_CODE "renderListComplete"
#define RENDER_LIST_END_CODE "renderListEnd"

void SendPlayerTickMsg(PlayerTickData data) {
	if (fre_context != nullptr) {
		stringstream ss;
		ss << "{\"played\": " << data.timestamp << ", \"duration\": " << data.duration << "}";
		const uint8_t* s = (const uint8_t*)ss.str().c_str();
		FREDispatchStatusEventAsync(fre_context, (const uint8_t*)PLAYER_TICK_CODE, (const uint8_t*)ss.str().c_str());
	}
}

void SendPlayerEndMsg(int flag) {
	if (fre_context != nullptr) {
		stringstream ss;
		ss << "{\"flag\": " << flag << "}";
		const uint8_t* s = (const uint8_t*)ss.str().c_str();
		FREDispatchStatusEventAsync(fre_context, (const uint8_t*)PLAYER_END_CODE, (const uint8_t*)ss.str().c_str());
	}
}

void SendRenderListProgressMsg(RenderMessage *progress) {
	if (fre_context != nullptr) {
		stringstream ss;
		ss << "{\"status\": [";
		for (auto i = 0; i < progress->count; i++) {
			ss << progress->status[i];
			if (i != progress->count - 1) {
				ss << ",";
			}
		}
		ss << "]}";
		FREDispatchStatusEventAsync(fre_context, (const uint8_t*)RENDER_LIST_PROGRESS_CODE, (const uint8_t*)ss.str().c_str());
	}
}

void SendRenderListEndMsg(RenderMessage *progress) {
	if (fre_context != nullptr) {
		stringstream ss;
		ss << "{\"status\": [";
		for (auto i = 0; i < progress->count; i++) {
			ss << progress->status[i];
			if (i != progress->count - 1) {
				ss << ",";
			}
		}
		ss << "]}";
		FREDispatchStatusEventAsync(fre_context, (const uint8_t*)RENDER_LIST_END_CODE, (const uint8_t*)(StringToUTF8(ss.str())).c_str());
	}
}

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

	if (!inited) {
		handler = GetModuleHandle(L"VoiceChanger.dll");
		if (handler != nullptr) {
			subscribe_player_tick = (void(*)(OnPlayerTick))GetProcAddress(handler, "SubscribePlayerTick");
			if (subscribe_player_tick == nullptr)return FromInt(LOAD_LIBRARY_FAILED);

			subscribe_player_end = (void(*)(OnPlayerEnd))GetProcAddress(handler, "SubscribePlayerEnd");
			if (subscribe_player_end == nullptr)return FromInt(LOAD_LIBRARY_FAILED);

			subscribe_render_list_progress = (void(*)(OnRenderListProgress))GetProcAddress(handler, "SubscribeRenderListProgress");
			if (subscribe_render_list_progress == nullptr)return FromInt(LOAD_LIBRARY_FAILED);

			subscribe_render_list_end = (void(*)(OnRenderListEnd))GetProcAddress(handler, "SubscribeRenderListEnd");
			if (subscribe_render_list_end == nullptr)return FromInt(LOAD_LIBRARY_FAILED);

			get_source_info = (void(*)(int*))GetProcAddress(handler, "GetSourceInfo");
			if (get_source_info == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			set_temp_path = (int(*)(const char*))GetProcAddress(handler, "SetExtractTempPath");
			if (set_temp_path == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			dispose_temps = (int(*)(void))GetProcAddress(handler, "DisposeAllTemps");
			if (dispose_temps == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			seek = (int(*)(int))GetProcAddress(handler, "Seek");
			if (seek == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			set_loop = (void(*)(bool))GetProcAddress(handler, "SetLoopPreview");
			if (set_loop == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			set_segments = (int(*)(int*, int))GetProcAddress(handler, "SetSegmentsPreview");
			if (set_segments == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
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
			cancel_render = (int(*)(const char*))GetProcAddress(handler, "CancelRender");
			if (cancel_render == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			cancel_render_all = (int(*)())GetProcAddress(handler, "CancelRenderAll");
			if (cancel_render_all == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			clear_render_list = (int(*)())GetProcAddress(handler, "ClearRenderList");
			if(clear_render_list == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			set_max_renderers_run_together = (int(*)(int))GetProcAddress(handler, "SetMaxRenderersRunTogether");
			if (set_max_renderers_run_together == nullptr)return FromInt(LOAD_LIBRARY_FAILED);
			// Subscribe event
			subscribe_player_tick(SendPlayerTickMsg);
			subscribe_player_end(SendPlayerEndMsg);
			subscribe_render_list_progress(SendRenderListProgressMsg);
			subscribe_render_list_end(SendRenderListEndMsg);
			inited = true;
		}else
			return FromInt(LOAD_LIBRARY_FAILED);
	}
	return FromInt(0);
}

FREObject GetSourceInfo(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	static int arr[5] = {0, 0, 0, 0, 0};
	arr[0] = arr[1] = arr[2] = arr[3] = arr[4] = 0;
	get_source_info(arr);
	stringstream ss;
	ss << "{\"sampleRate\": " << arr[0]
	   << ",\"bits\": " << arr[1] 
	   << ",\"channels\": " << arr[2]
	   << ",\"blockAlign\": " << arr[3]
	   << ",\"duration\": " << arr[4] << "}";
	return FromString(ss.str().c_str());
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
	return FromInt(set_effect(ToInt(argv[0])));
}

FREObject SetLoopPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	set_loop(ToBool(argv[0]));
	return FromInt(0);
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
	return FromInt(seek(ToInt(argv[0])));
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
	int* option_counts = ToIntArray(argv[4]);
	int* segments = ToIntArray(argv[5]);
	int* segment_counts = ToIntArray(argv[6]);
	int count = ToInt(argv[7]);
	if (count <= 0)goto param_not_match;
	return FromInt(render(sources, dests, effects, options, option_counts, segments, segment_counts, count));
param_not_match:
	return FromInt(PARAM_NOT_MATCH);
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

FREObject SetMaxRenderersRunTogether(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (argc < 1)return FromInt(MISS_PARAM);
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(set_max_renderers_run_together(ToInt(argv[0])));
}

FREObject ClearRenderList(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	if (!inited)return FromInt(HAS_NOT_INITED);
	return FromInt(clear_render_list());
}

void ContextInitializer(
	void					 * extData,
	const uint8_t			 * ctxType,
	FREContext				   ctx,
	uint32_t				 * numFunctionsToSet,
	const FRENamedFunction	** functionsToSet) {
	static FRENamedFunction fns[] = {
		{ (const uint8_t*) "getSourceInfo", NULL, &GetSourceInfo },
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
		{ (const uint8_t*) "cancelRender", NULL, &CancelRender },
		{ (const uint8_t*) "cancelRenderAll", NULL, &CancelRenderAll },
		{ (const uint8_t*) "setMaxRenderersRunTogether", NULL, &SetMaxRenderersRunTogether },
		{ (const uint8_t*) "clearRenderList", NULL, &ClearRenderList }
	};
	*numFunctionsToSet = sizeof(fns) / sizeof(FRENamedFunction);
	*functionsToSet = fns;
	fre_context = ctx;
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