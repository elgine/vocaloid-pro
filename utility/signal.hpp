#pragma once
#include <list>
#include <functional>
#include <string.h>
#include <algorithm>
using namespace std;

template<typename T>
class Signal {
private:
	list<function<void(T)>> handlers_;

public:
	explicit Signal() {}

	void On(function<void(T)> handler) {
		handlers_.push_back(handler);
	}

	void Off(function<void(T)> handler) {
		auto iter = find(handlers_.begin(), handlers_.end(), handler);
		if (iter != handlers_.end()) {
			handlers_.erase(iter);
		}
	}

	void Emit(T v) {
		for (auto handler : handlers_) {
			handler(v);
		}
	}
};