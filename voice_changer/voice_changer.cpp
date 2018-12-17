#include "voice_changer.h"

AudioContext *context = nullptr;

AudioContext *GetContext() {
	if (!context)context = new AudioContext();
	return context;
}

int Prepare(ConversationParams params[]) {
	// TODO: prepare conversation

	auto ctx = GetContext();
	return ctx->Prepare();
}

void Start() {
	auto ctx = GetContext();
	ctx->Start();
}

int Stop() {
	auto ctx = GetContext();
	return ctx->Stop();
}

int Close() {
	auto ctx = GetContext();
	return ctx->Close();
}