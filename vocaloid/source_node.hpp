#pragma once
#include "input_node.hpp"
#include "timeline.hpp"
namespace vocaloid {
	namespace node {
		class SourceNode : public InputNode, public Timeline{
		protected:			
			bool ready_to_play_;
			int64_t delay_;

			int64_t delay_pos_;
			int64_t delay_frames_;

			int64_t play_pos_;
		public:

			explicit SourceNode(BaseAudioContext *ctx) :InputNode(ctx) {
				loop_ = false;
				ready_to_play_ = false;
				delay_ = 0;
				delay_pos_ = 0;
				delay_frames_ = 0;
				play_pos_ = 0;
			}

			int Initialize(int32_t sample_rate, int64_t frame_size) override {
				InputNode::Initialize(sample_rate, frame_size);
				delay_frames_ = MsecToBytes(sample_rate, delay_);
				play_pos_ = 0;
				return SUCCEED;
			}

			void SetChannels(int16_t c) final {}

			virtual int64_t GetBuffer(int64_t offset, int64_t len) {
				return 0;
			}

			void Seek(int64_t timestamp) {
				auto frames = timestamp * 0.001f * sample_rate_;
				auto seg_index = Timeline::Seek(frames);
				if (seg_index > -1) {
					auto seg = Timeline::Segment(seg_index);
					if (frames < seg.start || frames >= seg.end) {
						Timeline::SetSegmentIndex(seg_index);
					}
					play_pos_ = SeekInternal(frames);
				}
			}

			virtual int64_t SeekInternal(int64_t frame_offset) { return frame_offset; }

			int64_t ProcessFrame() override {
				if (SegmentCount() <= 0)return EOF;
				auto frame_count = 0;
				auto frame_index = 0;
				auto silence_frames = 0;
				auto frame_size = frame_size_;
				if (!ready_to_play_){
					if (delay_frames_ > 0) {
						if (delay_pos_ + frame_size_ < delay_frames_) {
							delay_pos_ += frame_size_;
							return 0;
						}
						frame_size = (delay_pos_ + frame_size_) - delay_frames_;
						silence_frames = frame_index = delay_frames_ - delay_pos_;
					}
					ready_to_play_ = true;
					play_pos_ = Timeline::FirstSegment().start;
					SeekInternal(play_pos_);
				}

				int64_t byte_left = 0, count = 0;
				int64_t prev_segment_index = Timeline::Index(), cur_segment_index = Timeline::Index();
				TimeSegment last_segment = Timeline::LastSegment(), 
					first_segment = Timeline::FirstSegment(), 
					cur_segment = Segment(cur_segment_index);

				while (frame_count < frame_size) {
					if (Timeline::IsEnd() && play_pos_ >= last_segment.end) {
						if (loop_) {
							Timeline::SetSegmentIndex(0);
							play_pos_ += - last_segment.end + first_segment.start;
							cur_segment_index = 0;
							play_pos_ = SeekInternal(play_pos_);
						}
						else {
							return EOF;
						}
					}
					else {
						cur_segment_index = Timeline::Seek(play_pos_);
						if (prev_segment_index != cur_segment_index || cur_segment_index == -1) {
							play_pos_ -= cur_segment.end;
							cur_segment_index = Timeline::Next();
							if (cur_segment_index < 0)return EOF;
							cur_segment = Timeline::Segment(cur_segment_index);
							play_pos_ += cur_segment.start;
							// Post update
							play_pos_ = SeekInternal(play_pos_);
						}
					}
					cur_segment = Timeline::Segment(cur_segment_index);
					prev_segment_index = cur_segment_index;
					
					byte_left = cur_segment.end - play_pos_;
					count = min(frame_size - frame_count, byte_left);

					count = GetBuffer(frame_index, count);
					if (count <= 0) {
						if (count == EOF)return EOF;
						break;
					}
					frame_count += count;
					frame_index += count;
					play_pos_ += count;
				}
				return frame_count;
			}

			int StartWithSegments(int64_t **segments, int segment_count, int64_t delay = 0) {
				auto dur = int64_t(Duration() * SourceSampleRate() * 0.001f);
				if (dur <= 0)return HAVE_NOT_DEFINED_SOURCE;
				auto ret = InputNode::Start();
				if (ret < 0)return ret;
				delay_ = delay;
				try {
					int64_t last = 0, start_timestamp, end_timestamp, start_bytes = 0, end_bytes = 0;
					for (auto i = 0; i < segment_count; i++) {
						start_timestamp = Clamp(int64_t(0), dur, segments[i][0]);
						end_timestamp = Clamp(int64_t(0), dur, segments[i][1]);
						if (start_timestamp < last) {
							start_timestamp = last;
						}
						if (end_timestamp < start_timestamp) {
							end_timestamp = start_timestamp + 1;
						}
						last = end_timestamp;
						AddSegment(MsecToBytes(SourceSampleRate(), start_timestamp), MsecToBytes(SourceSampleRate(), end_timestamp));
					}
				}
				catch (exception e) {
					return INVALIDATE_SEGMENT_DATA;
				}
				return SUCCEED;
			}

			int Start(int64_t start = 0, int64_t duration = 0, int64_t delay = 0) {
				auto dur = Duration();
				if (dur <= 0)return HAVE_NOT_DEFINED_SOURCE;
				int64_t start_bytes = 0, end_bytes = 0;
				auto ret = InputNode::Start();
				if (ret < 0)return ret;
				delay_ = delay;
				start_bytes = MsecToBytes(SourceSampleRate(), Clamp(int64_t(0), Duration(), start));
				end_bytes = MsecToBytes(SourceSampleRate(), start + (duration <= 0 || (start + duration) > Duration()) ? (Duration() - start) : duration);
				Timeline::Dispose();
				AddSegment(start_bytes, end_bytes);
				return SUCCEED;
			}

			void Clear() override {
				Timeline::Clear();
				play_pos_ = 0;
				delay_pos_ = 0;
			}

			void Dispose() override {
				AudioNode::Dispose();
				Timeline::Dispose();
			}

			virtual int32_t SourceSampleRate() { return sample_rate_; }

			virtual int64_t Duration() { return 0; }
		};
	}
}