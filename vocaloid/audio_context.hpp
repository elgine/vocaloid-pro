#pragma once
#include "stdafx.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <iostream>
#include "audio_frame.hpp"
#include "audio_timeline.hpp"

namespace vocaloid {
	
	namespace node {
		int64_t PROCESSOR_UNIT_COUNTER = 0;
		enum AudioProcessorType {
			NORMAL,
			INPUT,
			OUTPUT
		};

		class AudioProcessorUnit {
		protected:
			int64_t id_;
			AudioProcessorType type_;
			bool can_be_connected_;
			bool can_connect_;
			set<AudioProcessorUnit*> inputs_;

			int16_t channels_;
			int64_t frame_size_;
			int32_t sample_rate_;
			AudioFrame *summing_buffer_;
			AudioFrame *result_buffer_;

			bool enable_;
		public:

			explicit AudioProcessorUnit(AudioProcessorType type,
										bool can_be_connected = true,
										bool can_connect = true) :
										id_(PROCESSOR_UNIT_COUNTER++),
										type_(type),
										can_be_connected_(can_be_connected),
										can_connect_(can_connect) {
				frame_size_ = DEFAULT_FRAME_SIZE;
				sample_rate_ = DEFAULT_SAMPLE_RATE;
				channels_ = 2;
				enable_ = true;
				summing_buffer_ = new AudioFrame(channels_, frame_size_);
				result_buffer_ = new AudioFrame(channels_, frame_size_);
			}

			virtual void Initialize(int32_t sample_rate, int64_t frame_size) {
				sample_rate_ = sample_rate;
				frame_size_ = frame_size;
				summing_buffer_->Alloc(channels_, frame_size_);
				summing_buffer_->SetSize(frame_size_);
				result_buffer_->Alloc(channels_, frame_size_);
				result_buffer_->SetSize(frame_size_);
			}

			virtual void SetChannels(int16_t c) {
				channels_ = c;
			}

			AudioFrame* Result() {
				return result_buffer_;
			}

			void PullBuffers() {
				summing_buffer_->Zero();
				for (auto input : inputs_) {
					summing_buffer_->Mix(input->Result());
				}
				result_buffer_->Copy(summing_buffer_);
			}

			virtual int64_t ProcessFrame() = 0;

			virtual void Close() {}

			virtual void Stop() {}

			virtual int64_t SuggestFrameSize() {
				return DEFAULT_FRAME_SIZE;
			}

			virtual int64_t Process() {
				PullBuffers();
				return ProcessFrame();
			}

			void ConnectFrom(AudioProcessorUnit* input) {
				if (can_be_connected_)
					inputs_.insert(input);
			}

			void DisconnectFrom(AudioProcessorUnit* input) {
				inputs_.erase(input);
			}

			set<AudioProcessorUnit*> Inputs() {
				return inputs_;
			}

			int64_t Id() {
				return id_;
			}

			AudioProcessorType Type() {
				return type_;
			}

			bool CanBeConnected() {
				return can_be_connected_;
			}

			bool CanConnect() {
				return can_connect_;
			}
		};
		
		class AudioGraph{
		protected:
			map<int64_t, AudioProcessorUnit*> nodes_;
			map<int64_t, set<int64_t>> connections_;
		public:

			explicit AudioGraph(){}

			void Traverse(vector<int64_t> &visited_nodes) {
				queue<int64_t> traverse_nodes;
				for (auto pair : nodes_) {
					if (pair.second->Type() == AudioProcessorType::OUTPUT) {
						traverse_nodes.push(pair.second->Id());
					}
				}
				while (!traverse_nodes.empty()) {
					auto node = traverse_nodes.front();
					traverse_nodes.pop();
					visited_nodes.insert(visited_nodes.begin(), node);
					auto dependencies = FindNode(node)->Inputs();
					for (auto dependency : dependencies) {
						if (find(visited_nodes.begin(), visited_nodes.end(), dependency->Id()) != visited_nodes.end())continue;
						auto all_output_visited = true;
						for (auto child : Children(dependency->Id())) {
							if (find(visited_nodes.begin(), visited_nodes.end(), child) == visited_nodes.end()) {
								all_output_visited = false;
								break;
							}
						}
						if (!all_output_visited)continue;
						traverse_nodes.push(dependency->Id());
					}
				}
			}

			set<int64_t> Children(int64_t id) {
				return connections_[id];
			}

			void AddNode(AudioProcessorUnit* node) {
				nodes_.insert(pair<int64_t, AudioProcessorUnit*>(node->Id(), node));
			}

			void RemoveNode(AudioProcessorUnit* node) {
				auto id = node->Id();
				nodes_.erase(id);
				for (auto dest : connections_[id]) {
					Disconnect(node, FindNode(dest));
				}
			}

			void Connect(AudioProcessorUnit *from_node, AudioProcessorUnit *to_node) {
				if (!from_node->CanConnect() || !to_node->CanBeConnected())return;
				if (nodes_.find(from_node->Id()) == nodes_.end())
					AddNode(from_node);
				if (nodes_.find(to_node->Id()) == nodes_.end())
					AddNode(to_node);
				auto from = from_node->Id();
				auto to = to_node->Id();
				if (connections_.find(from) == connections_.end()) {
					connections_[from] = {};
				};
				connections_[from].insert(to);
				to_node->ConnectFrom(from_node);
			}

			void Disconnect(AudioProcessorUnit *from_node, AudioProcessorUnit *to_node) {
				auto from = from_node->Id();
				auto to = to_node->Id();
				if(connections_.find(from) != connections_.end())
					connections_[from].erase(to);
				to_node->DisconnectFrom(from_node);
			}

			AudioProcessorUnit* FindNode(int64_t v_id) {
				return nodes_[v_id];
			}

			set<int64_t> FindConnection(int64_t v_id) {
				return connections_[v_id];
			}
		};

		enum AudioParamType {
			K_RATE,
			A_RATE
		};

		class AudioParam: public AudioProcessorUnit, public AudioTimeline {
		private:
			int64_t offset_;
		public:

			explicit AudioParam() :AudioProcessorUnit(AudioProcessorType::NORMAL, true, true),AudioTimeline(){
				offset_ = 0;
				channels_ = 1;
			}

			void SetChannels(int16_t channels) final {}

			void SetOffset(int64_t offset) {
				offset_ = offset;
			}

			void Offset(int64_t len) {
				offset_ += len;
			}

			int64_t ProcessFrame() {
				for (auto i = 0; i < frame_size_; i++) {
					result_buffer_->Channel(0)->Data()[i] = summing_buffer_->Channel(0)->Data()[i] + GetValueAtTime(CalculatePlayedTime(sample_rate_, offset_ + i));
				}
				offset_ += frame_size_;
				return frame_size_;
			}
		};

		enum AudioContextState {
			STOPPED,
			PLAYING
		};

		class AudioContext : public AudioGraph {
		private:
			atomic<AudioContextState> state_;
			unique_ptr<thread> audio_render_thread_;
			mutex audio_render_thread_mutex_;
			vector<int64_t> traversal_nodes_;
			int32_t sample_rate_;
			int64_t frame_size_;

			void Run() {
				while (state_ == AudioContextState::PLAYING) {
					{
						unique_lock<mutex> lck(audio_render_thread_mutex_);
						try {
							for (auto iter = traversal_nodes_.begin(); iter != traversal_nodes_.end(); iter++) {
								auto node = FindNode(*iter);
								node->Process();
							}
						}
						catch (exception e) {
							cout << e.what() << endl;
						}
					}
					this_thread::sleep_for(chrono::milliseconds(MINUS_SLEEP_UNIT));
				}
			}

		public:

			explicit AudioContext():state_(AudioContextState::STOPPED){
				sample_rate_ = DEFAULT_SAMPLE_RATE;
				frame_size_ = DEFAULT_FRAME_SIZE;
			}

			void Prepare() {
				Traverse(traversal_nodes_);
				int64_t frame_size = DEFAULT_FRAME_SIZE;
				for (auto node : traversal_nodes_) {
					frame_size = max(FindNode(node)->SuggestFrameSize(), frame_size);
				}
				frame_size_ = min((int64_t)MAX_FFT_SIZE, frame_size);
				for (auto node : traversal_nodes_) {
					FindNode(node)->Initialize(SampleRate(), frame_size_);
				}
			}

			void Start() {
				state_ = AudioContextState::PLAYING;
				audio_render_thread_ = make_unique<thread>(thread(&AudioContext::Run, this));
			}

			void Stop() {
				state_ = AudioContextState::STOPPED;
				for (auto node : traversal_nodes_) {
					FindNode(node)->Stop();
				}
				if (audio_render_thread_->joinable())
					audio_render_thread_->join();
			}

			void Close() {
				for (auto node : traversal_nodes_) {
					FindNode(node)->Close();
				}
			}

			int32_t SampleRate() {
				return sample_rate_;
			}
		};

		class AudioNode : public AudioProcessorUnit {
		protected:
			AudioContext *context_;

			void RegisterAudioParam(AudioParam *param) {
				context_->Connect(param, this);
			}
		public:
			explicit AudioNode(AudioContext *ctx,
				AudioProcessorType type = AudioProcessorType::NORMAL,
				bool can_be_connected = true,
				bool can_connect = true) :
				AudioProcessorUnit(type, can_be_connected, can_connect),
				context_(ctx) {
				context_->AddNode(this);
			}

			~AudioNode() {
				context_->RemoveNode(this);
			}
		};
	}
}