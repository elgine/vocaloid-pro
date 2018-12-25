#pragma once
#include "stdafx.h"
#include <thread>
#include <stack>
#include <mutex>
#include <atomic>
#include <iostream>
#include <algorithm>
#include "audio_graph.hpp"
#include "../utility/emitter.hpp"

namespace vocaloid {
	
	namespace node {
		enum AudioParamType {
			K_RATE,
			A_RATE
		};

		class AudioNode;

		class AudioParam : public AudioProcessorUnit, public AudioTimeline {
		private:
			int64_t offset_;
		public:

			explicit AudioParam() :AudioProcessorUnit(AudioProcessorType::PARAM, true, true), AudioTimeline() {
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

		class AudioContext : public AudioGraph,  public Emitter{
		public:
			static const char* END;
		private:
			atomic<AudioContextState> state_;
			unique_ptr<thread> audio_render_thread_;
			mutex audio_render_thread_mutex_;
			vector<int64_t> traversal_nodes_;
			int32_t sample_rate_;
			int64_t frame_size_;

			void Run() {
				bool all_input_eof = false;
				int64_t processed_frames = 0;
				while (state_ == AudioContextState::PLAYING) {
					{
						unique_lock<mutex> lck(audio_render_thread_mutex_);
						try {
							if(!all_input_eof)all_input_eof = true;
							for (auto iter = traversal_nodes_.begin(); iter != traversal_nodes_.end(); iter++) {
								auto node = FindNode(*iter);
								processed_frames = node->Process();
								if (processed_frames != EOF && node->Type() == AudioProcessorType::INPUT) {
									all_input_eof = false;	
								}							
							}
						}
						catch (exception e) {
							cout << e.what() << endl;
						}
					}
					this_thread::sleep_for(chrono::milliseconds(MINUS_SLEEP_UNIT));
				}
				if (all_input_eof) {
					Emit(END, nullptr);
				}
			}

		public:
			explicit AudioContext():AudioGraph(), Emitter(), state_(AudioContextState::STOPPED){
				sample_rate_ = DEFAULT_SAMPLE_RATE;
				frame_size_ = DEFAULT_FRAME_SIZE;
			}

			int Prepare() {
				Traverse(traversal_nodes_);
				/*int64_t frame_size = DEFAULT_FRAME_SIZE;
				for (auto node : traversal_nodes_) {
					frame_size = max(FindNode(node)->SuggestFrameSize(), frame_size);
				}
				frame_size_ = min((int64_t)MAX_FFT_SIZE, frame_size);*/
				for (auto node : traversal_nodes_) {
					FindNode(node)->Initialize(SampleRate(), frame_size_);
				}
				return 0;
			}

			void Start() {
				state_ = AudioContextState::PLAYING;
				audio_render_thread_ = make_unique<thread>(thread(&AudioContext::Run, this));
			}

			int Stop() {
				for (auto node : traversal_nodes_) {
					FindNode(node)->Stop();
				}
				state_ = AudioContextState::STOPPED;
				if (audio_render_thread_->joinable())
					audio_render_thread_->join();
				return 0;
			}

			int Close() {
				Stop();
				for (auto node : traversal_nodes_) {
					FindNode(node)->Close();
				}
				return 0;
			}

			void Reset() {
				for (auto node : nodes_) {
					node.second->Reset();
				}
			}

			void Dispose() {
				for (auto connection : connections_) {
					for (auto to : connection.second) {
						Disconnect(connection.first, to);
					}
					connection.second.clear();
				}
				connections_.clear();
				for (auto node : nodes_) {
					delete node.second;
					node.second = nullptr;
				}
				nodes_.clear();
			}

			int32_t SampleRate() {
				return sample_rate_;
			}
		};

		const char* AudioContext::END = "all_input_not_loop_finished";

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