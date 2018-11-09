#pragma once
#include "audio_context.hpp"
namespace vocaloid {
	namespace node {
		class SourceNode : public AudioNode {
		public:
			explicit SourceNode(AudioContext *ctx) :AudioNode(ctx, AudioProcessorType::INPUT, false, true) {}
		};
	}
}