#pragma once
#include "extract_resource.hpp"
#include "../utility/buffer.hpp"
#include "../vocaloid/file.h"
#include "../vocaloid/read_file_buffer.hpp"
#include "../vocaloid/audio_node.hpp"
#include "resource.h"

static auto buffer = new vocaloid::Buffer<char>();
static auto format = new vocaloid::io::AudioFormat();

void LoadKernel(int res_id, WCHAR* type, vocaloid::node::AudioChannel* channel_data) {
	buffer->SetSize(0);
	ReadFileBuffer(ExtractResource(res_id, type).data(), format, buffer);
	channel_data->FromBuffer(buffer, format->bits, format->channels);
}