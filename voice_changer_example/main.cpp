#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
typedef void(*SetExtractTempPath)(const char*);
typedef int(*DisposeTempOrPlayOrStopOrForceExitRender)();
typedef void(*Render)(const char** sources, const char **dest, int *effect_ids,
	float **options, int *option_count, int64_t*** segmentses, int *segment_counts, int count);


void main() {
	auto handler = LoadLibrary(L"VoiceChanger.dll");
	if (handler == nullptr)goto free;

	SetExtractTempPath set_extract_temp_path_fn = (SetExtractTempPath)GetProcAddress(handler, "SetExtractTempPath");
	if (set_extract_temp_path_fn == nullptr)goto free;

	DisposeTempOrPlayOrStopOrForceExitRender dispose_temp_fn = (DisposeTempOrPlayOrStopOrForceExitRender)GetProcAddress(handler, "DisposeAllTemps");
	if (dispose_temp_fn == nullptr)goto free;

	Render render_fn = (Render)GetProcAddress(handler, "Render");

	set_extract_temp_path_fn("C:\\Users\\Admin\\Desktop");
	/*open_fn("G:\\Projects\\VSC++\\vocaloid\\samples\\male-voice.mp3");
	set_effect_fn(15);
	
	play_fn();
	Sleep(500);
	set_loop_fn(true);
	getchar();
	stop_fn();*/
	const char** input = new const char*[2]{ 
		"D:\\Projects\\vocaloid\\samples\\example.mp3"
	};
	const char** output = new const char*[2]{
		"C:\\Users\\Admin\\Desktop\\output.mp3",
	};
	render_fn(input,
		output,
		new int[1]{ 21 },
		nullptr,
		nullptr,
		nullptr, nullptr, 1);
	getchar();
	dispose_temp_fn();
free:
	FreeLibrary(handler);
}