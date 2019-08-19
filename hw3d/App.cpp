#include "App.h"
#include <sstream>
#include <iomanip>
//using namespace std;

App::App() 
	: wnd(800, 600, "Lack of a better name") {
}

int App::Go() {
	while(true) {
		//process all pending messages, but do not block for new messages
		if(const auto ecode = Window::ProcessMessages()) {
			return *ecode;
		}
		DoFrame();
	}
}

void App::DoFrame(){
	const float c = sin(timer.Peek()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	wnd.Gfx().DrawTestTriangle(timer.Peek(), 0.0f, 0.0f, 7.0f);
	wnd.Gfx().DrawTestTriangle(timer.Peek(), 1.0f, 1.0f, 4.0f);
	wnd.Gfx().EndFrame();

	/*const float t = timer.Peek();
	ostringstream oss;
	oss << "Time elapsed: " << setprecision(1) << fixed << t << "s";
	wnd.SetTitle(oss.str());*/
}