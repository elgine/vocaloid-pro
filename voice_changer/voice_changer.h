#pragma once
#include "stdafx.h"
#include "effects.h"
#include "../vocaloid/audio_context.hpp"
using namespace vocaloid;
using namespace vocaloid::node;

struct ConversationParams {
	const char* source;
	const char* dest;
	Effect effect;
	void* effect_param;
};

AudioContext* GetContext();

int Prepare(ConversationParams params[]);

void Start();

int Stop();

int Close();