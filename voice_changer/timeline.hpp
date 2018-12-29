#pragma once
#include <stdint.h>
#include <vector>
using namespace std;

struct TimeSegment {
	int64_t start;
	int64_t end;
};

class Timeline {
private:
	vector<TimeSegment> segments_;
	int64_t cur_index_;
public:

	explicit Timeline() {}

	void Set(int64_t ** segments, int segment_count) {
		if (segment_count <= 0)return;
		auto segment_size = sizeof(segments[0]) / sizeof(int64_t);
		if (segment_size <= 1)return;
		Dispose();
		for (auto i = 0; i < segment_count; i++) {
			segments_.push_back({ segments[i][0], segments[i][1] });
		}
	}

	TimeSegment Seek(int64_t timestamp) {
		int64_t index = -1;
		if (timestamp < segments_[cur_index_].start || timestamp >= segments_[cur_index_].end) {
			int64_t start = 0, end = segments_.size() - 1, middle = 0;
			while (start <= end) {
				middle = (start + end) / 2;
				if (timestamp >= segments_[middle].start && timestamp < segments_[middle].end) {
					index = middle;
					break;
				}
				else if (timestamp >= segments_[middle].end) {
					start = middle + 1;
				}
				else {
					end = middle - 1;
				}
			}
		}
		if (index < 0) {
			return{
				-1, -1
			};
		}
		return segments_[index];
	}

	void SetSegmentIndex(int64_t index) {
		if (index < 0 || index >= segments_.size())return;
		cur_index_ = index;
	}

	int Next() {
		if (cur_index_ + 1 >= segments_.size())return -1;
		cur_index_++;
		return cur_index_;
	}

	int Prev() {
		if (cur_index_ - 1 < 0)return -1;
		cur_index_--;
		return cur_index_;
	}

	void Reset() {
		cur_index_ = 0;
	}

	void Dispose() {
		cur_index_ = 0;
		segments_.clear();
	}

	int64_t Index() {
		return cur_index_;
	}
};