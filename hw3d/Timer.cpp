#include "Timer.h"

Timer::Timer() {
	last = chrono::steady_clock::now();
}

float Timer::Mark() {
	const auto old = last;
	last = chrono::steady_clock::now();
	const chrono::duration<float> frameTime = last - old;
	return frameTime.count();
}

float Timer::Peek() const {
	return chrono::duration<float>(chrono::steady_clock::now() - last) .count();
}