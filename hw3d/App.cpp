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
	wnd.Gfx().DrawTestTriangle(-5.0f*(timer.Peek()));
	wnd.Gfx().EndFrame();

	/*const float t = timer.Peek();
	ostringstream oss;
	oss << "Time elapsed: " << setprecision(1) << fixed << t << "s";
	wnd.SetTitle(oss.str());*/
}