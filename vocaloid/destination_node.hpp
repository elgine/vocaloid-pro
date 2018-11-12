#pragma once
#include "stdafx.h"
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class DestinationNode: public AudioNode{
		protected:
			int32_t sample_rate_ = 44100;
		public:
			explicit DestinationNode(AudioContext *ctx) :AudioNode(ctx, AudioProcessorType::OUTPUT) {
				can_connect_ = false;
			}
		};
	}
}