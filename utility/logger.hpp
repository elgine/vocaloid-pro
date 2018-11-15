#pragma once
#include <fstream>
using namespace std;
class Logger {
private:
	std::ofstream stream_;
	static Logger* instance_;
public:

	static void Open(const char* path) {
		
		if (instance_ == nullptr)
			instance_ = new Logger(path);
	}

	static Logger* Instance() {
		
		return instance_;
	}

	Logger(const char* path) {
		stream_.open(path, ios::app);
	}

	void Write(const char) {
		
	}
};

Logger* Logger::instance_ = nullptr;

#define START_LOG(path)

#define LOG(x)\
	{
		
	}