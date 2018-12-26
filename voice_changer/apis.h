#pragma once
#include "effects.h"

// Play source in selected effect
void Preview(const char* source, int32_t effect_id, int64_t** segments);

// Record 
void Record(const char** sources, int32_t* effect_ids, int64_t*** segments, int count);