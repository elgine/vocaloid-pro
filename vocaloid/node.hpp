#pragma once
#include "unit.hpp"
namespace vocaloid {
	namespace node {
		class AudioContext;
		class Node : public Unit {
		protected:
			AudioContext *context_;
		public:
			static const uint16_t BITS_PER_SEC = 16;
			explicit Node(AudioContext *ctx) :Unit(), context_(ctx) {}
		};
	}
}