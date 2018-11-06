#pragma once
#include "node.hpp"
namespace vocaloid {
	namespace node {
		class SourceNode : public Node {
		public:
			explicit SourceNode(AudioContext *ctx) :Node(ctx) {
				can_be_connected_ = false;
			}

			void Pull(Frame *in) final {
				in->Alloc(channels_, frame_size_);
				Process(in);
			}
		};
	}
}