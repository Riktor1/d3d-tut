#pragma once
#include "Window.h"
#include "Timer.h"
#include <sstream>
//using namespace std;

class App {
public:
	App(); //creates window
	//Master frame/message loop
	int Go(); //called when app starts to start game loop
private:
	void DoFrame();
	Window wnd;
	Timer timer;
};