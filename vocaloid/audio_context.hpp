#pragma once
#include <atomic>
#include <thread>
#include "stdafx.h"
#include "audio_node.hpp"
#include "input_node.hpp"
#include "destination_node.hpp"
#include "player_node.hpp"
#include "file_writer_node.hpp"
#include "../utility/signal.hpp"
using namespace std;
namespace vocaloid {
	namespace node {
		class AudioContext: public BaseAudioContext{
		protected:
			map<int64_t, AudioNode*> nodes_;
			map<int64_t, set<int64_t>> connections_;
			DestinationNode *destination_;
			atomic<AudioContextState> state_;
			thread* audio_render_thread_;
			mutex audio_render_thread_mutex_;
			vector<int64_t> traversal_nodes_;
			int64_t frame_size_;

			atomic<bool> source_eof_;

			void FindCircleEnd(int64_t id, vector<int64_t> &path, map<int64_t, set<int64_t>> &circles) {
				if (!path.empty()) {
					// Exist circle
					if (id == path[0]) {
						circles[id].insert(path[path.size() - 1]);
						path.pop_back();
						return;
					}
					else if (find(path.begin(), path.end(), id) != path.end()) {
						return;
					}
				}

				for (auto child : Children(id)) {
					path.push_back(id);
					FindCircleEnd(child, path, circles);
				}
			}

			void FindCircleEnds(int64_t id, map<int64_t, set<int64_t>> &circles) {
				vector<int64_t> path;
				FindCircleEnd(id, path, circles);
			}

			enum VisitColor {
				WHITE,
				GREY,
				BLACK
			};

			void FindConnectedNodes(vector<int64_t> &visited_nodes) {
				vector<int64_t> traverse_nodes;
				map<int64_t, VisitColor> colors;
				for (auto pair : nodes_) {
					colors[pair.first] = VisitColor::WHITE;
					if (pair.second->Type() == AudioNodeType::OUTPUT) {
						traverse_nodes.emplace_back(pair.second->Id());
						colors[pair.first] = VisitColor::GREY;
					}
				}
				while (!traverse_nodes.empty()) {
					auto node = traverse_nodes.front();
					traverse_nodes.erase(traverse_nodes.begin());
					visited_nodes.insert(visited_nodes.begin(), node);
					colors[node] = VisitColor::BLACK;
					auto dependencies = FindNode(node)->Inputs();
					for (auto dependency : dependencies) {
						if (colors[dependency->Id()] != VisitColor::WHITE)continue;
						traverse_nodes.emplace_back(dependency->Id());
						colors[dependency->Id()] = VisitColor::GREY;
					}
				}
			}

			void Traverse(vector<int64_t> &visited_nodes) {
				vector<int64_t> nodes;
				FindConnectedNodes(nodes);
				queue<int64_t> traverse_nodes;
				map<int64_t, set<int64_t>> circle_ends;
				map<int64_t, VisitColor> colors;
				for (auto node : nodes) {
					colors[node] = VisitColor::WHITE;
					if (FindNode(node)->Inputs().empty()) {
						colors[node] = VisitColor::GREY;
						traverse_nodes.push(node);
					}
					FindCircleEnds(node, circle_ends);
				}
				while (!traverse_nodes.empty()) {
					auto node = traverse_nodes.front();
					traverse_nodes.pop();
					visited_nodes.push_back(node);
					colors[node] = VisitColor::BLACK;
					auto children = Children(node);
					for (auto child : children) {
						auto child_node = FindNode(child);
						if (colors[child] == VisitColor::BLACK)continue;
						auto all_visited = true;
						for (auto input : child_node->Inputs()) {
							if (colors[input->Id()] != VisitColor::BLACK && circle_ends[child].find(input->Id()) == circle_ends[child].end()) {
								all_visited = false;
								break;
							}
						}
						if (all_visited) {
							colors[child] = VisitColor::GREY;
							traverse_nodes.push(child);
						}
					}
				}
			}

			void Run() {
				bool all_input_eof = false;
				int64_t processed_frames_ = 0, cur_processed_frames = 0;
				int64_t sleep = destination_ && destination_->OutputType() == OutputType::PLAYER ? MINUS_SLEEP_UNIT : 2;
				while (true) {
					{
						unique_lock<mutex> lck(audio_render_thread_mutex_);
						if (state_ != AudioContextState::PLAYING)break;
						if (!all_input_eof)all_input_eof = true;
						for (auto iter = traversal_nodes_.begin(); iter != traversal_nodes_.end(); iter++) {
							auto node = FindNode(*iter);
							cur_processed_frames = node->Process();
							if (node->Type() == AudioNodeType::INPUT) {
								auto in_node = static_cast<InputNode*>(node);
								if (in_node->watched_ && (in_node->loop_ || (!in_node->loop_ && cur_processed_frames != EOF))) {
									all_input_eof = false;
								}
							}
						}
						if (all_input_eof) {
							destination_->Flush();
							source_eof_ = all_input_eof;
						}
					}
					if (!all_input_eof) {
						processed_frames_ += cur_processed_frames;
					}
					on_tick_->Emit(processed_frames_);
					this_thread::sleep_for(chrono::milliseconds(sleep));
					if (source_eof_) {
						on_end_->Emit(0);
						if (stop_eof_)break;
					}
				}
			}

		public:

			Signal<int64_t> *on_tick_;
			Signal<int> *on_end_;

			atomic<bool> stop_eof_;

			explicit AudioContext(){
				destination_ = nullptr;
				state_ = AudioContextState::STOPPED;
				frame_size_ = DEFAULT_FRAME_SIZE;
				audio_render_thread_ = nullptr;
				on_tick_ = new Signal<int64_t>();
				on_end_ = new Signal<int>();
				stop_eof_ = false;
				source_eof_ = true;
			}

			void SetOutput(OutputType output, int32_t sample_rate = 44100, int16_t channels = 2) override {
				vector<AudioNode*> inputs;
				{
					if (destination_ == nullptr) {
						if (output == OutputType::PLAYER) {
							destination_ = new PlayerNode(this);
						}
						else {
							destination_ = new FileWriterNode(this);
						}
					}
					else if (destination_->OutputType() != output) {
						inputs.assign(destination_->Inputs().begin(), destination_->Inputs().end());
						destination_->Dispose();
						delete destination_;
						destination_ = nullptr;
						if (output == OutputType::PLAYER) {
							destination_ = new PlayerNode(this);
						}
						else {
							destination_ = new FileWriterNode(this);
						}
					}
				}
				for (auto input : inputs) {
					Connect(input, destination_);
				}
				SetOutputFormat(sample_rate, channels);
			}

			void SetOutputFormat(int32_t sample_rate = 44100, int16_t channels = 2) override {
				if(destination_ != nullptr)
					destination_->SetFormat(sample_rate, channels);
			}

			set<int64_t> Children(int64_t id) override {
				return connections_[id];
			}

			void AddNode(AudioNode* node) override {
				nodes_[node->Id()] = node;
			}

			void RemoveNode(AudioNode* node) override {
				auto id = node->Id(); 
				for (auto f_node : node->Inputs()) {
					Disconnect(f_node, node);
				}
				node->Inputs().clear();
				auto connection = connections_[id];
				for (auto it = connection.begin(); it != connection.end();) {
					auto t_id = *it;
					it++;
					Disconnect(id, t_id);
				}
				nodes_.erase(id);
			}

			void Connect(AudioNode *from_node, AudioNode *to_node, 
						Channel from_channel = Channel::ALL, Channel to_channel = Channel::ALL) override {
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
				to_node->ConnectFrom(from_node, from_channel, to_channel);
			}

			void Disconnect(AudioNode *from_node, AudioNode *to_node, 
							Channel from_channel = Channel::ALL, Channel to_channel = Channel::ALL) override {
				auto from = from_node->Id();
				auto to = to_node->Id();
				if (connections_.find(from) != connections_.end())
					connections_[from].erase(to);
				to_node->DisconnectFrom(from_node);
			}

			void Disconnect(int64_t from, int64_t to) override {
				auto to_node = nodes_[to];
				auto from_node = nodes_[from];
				if (connections_.find(from) != connections_.end())
					connections_[from].erase(to);
				to_node->DisconnectFrom(from_node);
			}

			AudioNode* FindNode(int64_t v_id) override {
				return nodes_[v_id];
			}

			set<int64_t> FindConnection(int64_t v_id) override {
				return connections_[v_id];
			}

			void Lock() override {
				audio_render_thread_mutex_.lock();
			}

			void Unlock() override {
				audio_render_thread_mutex_.unlock();
			}

			int Prepare() override {
				unique_lock<mutex> lck(audio_render_thread_mutex_);
				source_eof_ = true;
				int ret = 0;
				traversal_nodes_.clear();
				Traverse(traversal_nodes_);
				for (auto node : traversal_nodes_) {
					ret = FindNode(node)->Initialize(SampleRate(), frame_size_);
					if (ret < 0)return ret;
				}
				return SUCCEED;
			}

			void Start() override {
				unique_lock<mutex> lck(audio_render_thread_mutex_);
				if (state_ == AudioContextState::PLAYING) {
					return;
				}
				source_eof_ = false;
				state_ = AudioContextState::PLAYING;
				if (audio_render_thread_ == nullptr) {
					audio_render_thread_ = new thread(thread(&AudioContext::Run, this));
				}
			}

			int Stop() override {
				{
					unique_lock<mutex> lck(audio_render_thread_mutex_);
					for (auto node : traversal_nodes_) {
						FindNode(node)->Stop();
					}
				}
				state_ = AudioContextState::STOPPED;
				if (audio_render_thread_ && audio_render_thread_->joinable()) {
					audio_render_thread_->join();
					delete audio_render_thread_;
					audio_render_thread_ = nullptr;
				}	
				return 0;
			}

			void Clear() override {
				{
					unique_lock<mutex> lck(audio_render_thread_mutex_);
					for (auto node : traversal_nodes_) {
						FindNode(node)->Clear();
					}
				}
			}

			void Close() override {
				if (destination_) {
					destination_->Close();
				}
			}

			void Dispose() override {
				Stop();
				Close();
				for (auto node : nodes_) {
					node.second->Dispose();
					delete node.second;
					node.second = nullptr;
				}
				nodes_.clear();
			}

			bool SourceEof() {
				unique_lock<mutex> lck(audio_render_thread_mutex_);
				return source_eof_;
			}

			AudioContextState State() {
				unique_lock<mutex> lck(audio_render_thread_mutex_);
				return state_;
			}

			int32_t SampleRate() override {
				return destination_->SampleRate();
			}

			DestinationNode* Destination() override {
				return destination_;
			}
		};
	}
}