#pragma once
#include "stdafx.h"
#include "frame.hpp"
namespace vocaloid {
	namespace node {	
		class Unit {
		protected:
			vector<Unit*> inputs_;
			uint16_t num_input_nodes_;
			uint16_t num_output_nodes_;
			vector<Unit*> outputs_;
			Frame *input_buffer_;
			Frame *summing_buffer_;
			uint64_t frame_size_;
			uint16_t channels_;
			bool enable_;
			bool can_be_connected_;
			bool can_connect_;

			bool new_frame_;
		public:
			explicit Unit(bool can_be_connected = true, bool can_connect = true,
				uint16_t channels = 2, uint64_t frame_size = 1024) :can_be_connected_(can_be_connected),
				can_connect_(can_connect),
				channels_(channels),
				frame_size_(frame_size) {
				enable_ = true;
				num_input_nodes_ = 0;
				num_output_nodes_ = 0;
				input_buffer_ = new Frame(channels_, frame_size_);
				summing_buffer_ = new Frame(channels_, frame_size_);
			}

			virtual void Initialize(uint64_t frame_size) {
				frame_size_ = frame_size;
				summing_buffer_->Alloc(channels_, frame_size_);
				summing_buffer_->SetSize(frame_size_);
				input_buffer_->SetSize(frame_size_);
				for (auto input : inputs_) {
					input->Initialize(frame_size);
				}
			}

			virtual void SetChannels(uint16_t c) {
				channels_ = c;
			}

			void SetEnable(bool v) {
				enable_ = v;
			}

			bool HasConnectedFrom(const Unit *n) {
				return find(inputs_.begin(), inputs_.end(), n) != inputs_.end();
			}

			bool HasConnectedTo(const Unit *n) {
				return find(outputs_.begin(), outputs_.end(), n) != outputs_.end();
			}

			bool CanProcess(uint64_t frame_size) {
				return summing_buffer_->Size() >= frame_size;
			}

			virtual int64_t Process(Frame *in) { return 0; }

			void PullInputs() {
				summing_buffer_->Fill(0.0f);
				if (!inputs_.empty()) {
					for (const auto &input : inputs_) {
						input->Pull(input_buffer_);
						summing_buffer_->Mix(input_buffer_);
					}
				}
			}

			virtual void Pull(Frame *in) {
				PullInputs();
				in->Alloc(channels_, frame_size_);
				in->SetSize(frame_size_);
				Process(in);
			}

			virtual void ConnectInput(Unit *n) {
				if (!can_be_connected_ || HasConnectedFrom(n))return;
				inputs_.emplace_back(n);
				num_input_nodes_++;
			}

			virtual void ConnectOutput(Unit *n) {
				if (!can_connect_ || HasConnectedTo(n))return;
				outputs_.emplace_back(n);
				num_output_nodes_++;
			}

			virtual void DisconnectInput(const Unit *n) {
				auto iter = find(inputs_.begin(), inputs_.end(), n);
				if (iter != inputs_.end()) {
					inputs_.erase(iter);
					num_input_nodes_--;
				}
			}

			virtual void DisconnectOutput(const Unit *n) {
				auto iter = find(outputs_.begin(), outputs_.end(), n);
				if (iter != outputs_.end()) {
					outputs_.erase(iter);
					num_output_nodes_--;
				}
			}

			virtual void Connect(Unit *n) {
				n->ConnectInput(this);
				ConnectOutput(n);
			}

			virtual void Disconnect(Unit *n) {
				DisconnectOutput(n);
				n->DisconnectInput(this);
			}

			uint16_t NumOutputNodes() {
				return num_output_nodes_;
			}

			vector<Unit*> Outputs() {
				return outputs_;
			}

			bool Enable() {
				return enable_;
			}

			uint16_t Channels() {
				return channels_;
			}

			uint16_t NumInputNodes() {
				return num_input_nodes_;
			}

			vector<Unit*> Inputs() {
				return inputs_;
			}
		};
	}
}