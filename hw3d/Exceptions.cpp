#include "UrielException.h"
#include <sstream>

using namespace std;

UrielException::UrielException(int line, const char* file) noexcept
	: line(line), file(file) {
}

const char* UrielException::what() const noexcept {
	ostringstream oss;
	oss << GetType() << endl
		<< GetOriginalString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* UrielException::GetType() const noexcept {
	return "Uriel Exception";
}

int UrielException::GetLine() const noexcept {
	return line;
}

const string& UrielException::GetFile() const noexcept {
	return file;
}

string UrielException::GetOriginalString() const noexcept {
	ostringstream oss;
	oss << "[File] " << file << endl
		<< "[Line] " << line;
	return oss.str();
}