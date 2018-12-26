#pragma once
#include "stdafx.h"
#include "channel.h"
namespace vocaloid {
	namespace node {
		enum AudioContextState {
			STOPPED,
			PLAYING
		};

		class AudioNode;
		class DestinationNode;
		class BaseAudioContext {
		public:
			virtual set<int64_t> Children(int64_t id) = 0;
			virtual void AddNode(AudioNode* node) = 0;
			virtual void RemoveNode(AudioNode* node) = 0;
			virtual void Connect(AudioNode *from_node, AudioNode *to_node, Channel from_channel = Channel::ALL, Channel to_channel = Channel::ALL) = 0;
			virtual void Disconnect(AudioNode *from_node, AudioNode *to_node, Channel from_channel = Channel::ALL, Channel to_channel = Channel::ALL) = 0;
			virtual void Disconnect(int64_t from, int64_t to) = 0;
			virtual AudioNode* FindNode(int64_t v_id) = 0;
			virtual set<int64_t> FindConnection(int64_t v_id) = 0;
			virtual void Lock() = 0;
			virtual void Unlock() = 0;
			virtual int Prepare() = 0;
			virtual void Start() = 0;
			virtual int Stop() = 0;
			virtual int Close() = 0;
			virtual void Reset() = 0;
			virtual void Dispose() = 0;
			virtual void SetOutput(OutputType output, int32_t sample_rate, int16_t channels) = 0;
			virtual void SetOutputFormat(int32_t sample_rate, int16_t channels) = 0;
			virtual AudioContextState State() = 0;
			virtual int32_t SampleRate() = 0;
			virtual DestinationNode* Destination() = 0;
		};
	}
}