#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
typedef void(*SetExtractTempPath)(const char*);
typedef int(*SetPreviewEffect)(int);
typedef void(*SetPreviewLoop)(bool);
typedef int(*OpenPreview)(const char*);
typedef void(*SetPreviewOptions)(float* options, int count);
typedef int(*DisposeTempOrPlayOrStopOrForceExitRender)();
typedef void(*Render)(const char** sources, const char **dest, int *effect_ids,
	float **options, int *option_count, int count,
	int64_t*** segmentses, int *segment_counts);


void main() {
	auto handler = LoadLibrary(L"voice_changer.dll");
	if (handler == nullptr)goto free;

	SetExtractTempPath set_extract_temp_path_fn = (SetExtractTempPath)GetProcAddress(handler, "SetExtractTempPath");
	if (set_extract_temp_path_fn == nullptr)goto free;

	DisposeTempOrPlayOrStopOrForceExitRender dispose_temp_fn = (DisposeTempOrPlayOrStopOrForceExitRender)GetProcAddress(handler, "DisposeTemp");
	if (dispose_temp_fn == nullptr)goto free;

	SetPreviewEffect set_effect_fn = (SetPreviewEffect)GetProcAddress(handler, "SetPreviewEffect");
	if (set_effect_fn == nullptr)goto free;

	SetPreviewLoop set_loop_fn = (SetPreviewLoop)GetProcAddress(handler, "SetPreviewLoop");

	OpenPreview open_fn = (OpenPreview)GetProcAddress(handler, "OpenPreview");
	if (open_fn == nullptr)goto free;

	DisposeTempOrPlayOrStopOrForceExitRender play_fn = (DisposeTempOrPlayOrStopOrForceExitRender)GetProcAddress(handler, "StartPreview");
	if (play_fn == nullptr)goto free;

	DisposeTempOrPlayOrStopOrForceExitRender stop_fn = (DisposeTempOrPlayOrStopOrForceExitRender)GetProcAddress(handler, "StopPreview");
	if (stop_fn == nullptr)goto free;

	Render render_fn = (Render)GetProcAddress(handler, "Render");
	if (stop_fn == nullptr)goto free;

	DisposeTempOrPlayOrStopOrForceExitRender exit_fn = (DisposeTempOrPlayOrStopOrForceExitRender)GetProcAddress(handler, "ForceExitRender");
	if (exit_fn == nullptr)goto free;

	set_extract_temp_path_fn("C:\\Users\\Elgine\\Desktop");
	/*open_fn("G:\\Projects\\VSC++\\vocaloid\\samples\\male-voice.mp3");
	set_effect_fn(15);
	
	play_fn();
	Sleep(500);
	set_loop_fn(true);
	getchar();
	stop_fn();*/
	const char** input = new const char*[2]{ 
		"G:\\Projects\\VSC++\\vocaloid\\samples\\female-voice.wav",
		"G:\\Projects\\VSC++\\vocaloid\\samples\\male-voice.mp3"
	};
	const char** output = new const char*[2]{
		"C:\\Users\\Elgine\\Desktop\\female-hall.mp3",
		"C:\\Users\\Elgine\\Desktop\\male-hall.mp3"
	};
	render_fn(input,
		output,
		new int[2]{ 15, 15 },
		nullptr,
		nullptr,
		2, nullptr, 0);
	getchar();
	exit_fn();
	dispose_temp_fn();
free:
	FreeLibrary(handler);
}