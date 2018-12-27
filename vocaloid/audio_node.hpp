#pragma once
#include "base_audio_context.h"
#include "audio_channel.hpp"
#include "status.h"
namespace vocaloid {
	namespace node {

		enum AudioNodeType {
			NORMAL,
			INPUT,
			OUTPUT,
			PARAM,
			SPLITTER,
			MERGER
		};

		struct AudioPort {
			Channel from_channel;
			Channel to_channel;
		};

		class AudioNode {
		private:
			static int64_t ID_COUNTER;
		protected:
			int64_t id_;
			BaseAudioContext *context_;
			AudioNodeType type_;
			bool can_be_connected_;
			bool can_connect_;
			set<AudioNode*> inputs_;
			map<int64_t, AudioPort> input_ports_;

			int16_t channels_;
			int64_t frame_size_;
			int32_t sample_rate_;
			AudioChannel *summing_buffer_;
			AudioChannel *result_buffer_;

		public:

			bool enable_;

			explicit AudioNode(BaseAudioContext *ctx, AudioNodeType type = AudioNodeType::NORMAL,
				bool can_be_connected = true,
				bool can_connect = true) :
				context_(ctx),
				id_(ID_COUNTER++),
				type_(type),
				can_be_connected_(can_be_connected),
				can_connect_(can_connect) {
				frame_size_ = DEFAULT_FRAME_SIZE;
				sample_rate_ = DEFAULT_SAMPLE_RATE;
				channels_ = 2;
				enable_ = true;
				summing_buffer_ = new AudioChannel(channels_, frame_size_);
				result_buffer_ = new AudioChannel(channels_, frame_size_);
			}

			virtual void ConnectFrom(AudioNode* input, Channel from, Channel to) {
				if (can_be_connected_) {
					inputs_.insert(input);
					input_ports_[input->id_] = { from, to };
				}
			}

			virtual void DisconnectFrom(AudioNode* input) {
				inputs_.erase(input);
				input_ports_.erase(input->id_);
			}

			virtual int Initialize(int32_t sample_rate, int64_t frame_size) {
				sample_rate_ = sample_rate;
				frame_size_ = frame_size;
				summing_buffer_->Alloc(channels_, frame_size_);
				summing_buffer_->SetSize(frame_size_);
				result_buffer_->Alloc(channels_, frame_size_);
				result_buffer_->SetSize(frame_size_);
				return SUCCEED;
			}

			virtual void SetChannels(int16_t c) {
				channels_ = c;
			}

			AudioChannel* GetResult() {
				return result_buffer_;
			}

			virtual void PullBuffers() {
				summing_buffer_->Zero();
				for (auto input : inputs_) {
					if (input->type_ != AudioNodeType::PARAM) {
						if (input->type_ == AudioNodeType::SPLITTER) {
							auto input_buf = input->GetResult()->Channel(input_ports_[input->id_].from_channel)->Data();
							auto output_buf = summing_buffer_->Channel(0)->Data();
							for (auto i = 0; i < frame_size_; i++) {
								output_buf[i] += input_buf[i];
							}
						}
						else if (input->type_ == AudioNodeType::MERGER) {
							auto input_buf = input->GetResult()->Channel(0)->Data();
							auto output_buf = summing_buffer_->Channel(input_ports_[input->id_].to_channel)->Data();
							for (auto i = 0; i < frame_size_; i++) {
								output_buf[i] += input_buf[i];
							}
						}
						else {
							summing_buffer_->Mix(input->GetResult());
						}

					}
				}
				result_buffer_->Copy(summing_buffer_);
			}

			virtual int64_t ProcessFrame() { return 0; }

			virtual void Dispose() {
				summing_buffer_->Dispose();
				delete summing_buffer_;
				summing_buffer_ = nullptr;

				result_buffer_->Dispose();
				delete result_buffer_;
				result_buffer_ = nullptr;
				context_->RemoveNode(this);
			}

			virtual void Stop() {}

			virtual void Clear() {}

			virtual void Reset() {}

			virtual int64_t Process() {
				PullBuffers();
				if (enable_)
					return ProcessFrame();
				return 0;
			}

			set<AudioNode*> Inputs() {
				return inputs_;
			}

			int64_t Id() {
				return id_;
			}

			AudioNodeType Type() {
				return type_;
			}

			bool CanBeConnected() {
				return can_be_connected_;
			}

			bool CanConnect() {
				return can_connect_;
			}
		};

		int64_t AudioNode::ID_COUNTER = 0;
	}
}