#pragma once
#include "audio_context.hpp"
#include <functional>
using namespace std;
namespace vocaloid {
	namespace node {
		class ScriptNode : public AudioNode {
		public:

			int64_t buffer_size_;

			function<int64_t(AudioChannel*, AudioChannel*)> process_fn_;

			int64_t ProcessFrame() override {
				return process_fn_(summing_buffer_, result_buffer_);
			}
		};
	}
}