#pragma once
#include <chrono>
using namespace std;

class Timer {
public:
	Timer();
	float Mark();
	float Peek() const;
private:
	std::chrono::steady_clock::time_point last;
};