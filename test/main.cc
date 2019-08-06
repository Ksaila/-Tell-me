#include "Window.hpp"
#include <unistd.h>

int main()
{
    Window w;
    w.DrawHeader();
    w.DrawOutput();
    w.DrawOnline();
    w.DrawInput();
    sleep(1);

    //w.Welcome(); 
    std::string message;
    for(;;){
	w.GetStringFromInput(message);
	
  	w.PutMessageToOutput(message);
    }
    return 0;
}
