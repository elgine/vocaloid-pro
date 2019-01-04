#include <Windows.h>
#include <stdio.h>
typedef void(*SetExtractTempPath)(const char*);
typedef int(*SetEffect)(int);
typedef void(*SetLoop)(bool);
typedef int(*Open)(const char*);
typedef void(*SetOptions)(float* options, int count);
typedef int(*PlayOrStop)();

void main() {
	auto handler = LoadLibrary(L"voice_changer.dll");
	if (handler == nullptr)goto free;

	SetExtractTempPath set_extract_temp_path_fn = (SetExtractTempPath)GetProcAddress(handler, "SetExtractTempPath");
	if (set_extract_temp_path_fn == nullptr)goto free;

	SetEffect set_effect_fn = (SetEffect)GetProcAddress(handler, "SetEffect");
	if (set_effect_fn == nullptr)goto free;

	SetLoop set_loop_fn = (SetLoop)GetProcAddress(handler, "SetLoop");

	Open open_fn = (Open)GetProcAddress(handler, "Open");
	if (open_fn == nullptr)goto free;

	PlayOrStop play_fn = (PlayOrStop)GetProcAddress(handler, "Play");
	if (play_fn == nullptr)goto free;

	PlayOrStop stop_fn = (PlayOrStop)GetProcAddress(handler, "StopPlaying");
	if (stop_fn == nullptr)goto free;

	set_extract_temp_path_fn("C:\\Users\\Elgine\\Desktop");
	open_fn("G:\\Projects\\VSC++\\vocaloid\\samples\\speech.wav");
	set_effect_fn(15);
	set_loop_fn(true);
	play_fn();
	getchar();
	stop_fn();
free:
	FreeLibrary(handler);
}