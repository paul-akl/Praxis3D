
#include "Engine.h"

int main(int argc, char* argv[])
{
	Engine engineInstance;

	if(engineInstance.init() == ErrorCode::Success)
	{
		engineInstance.run();
	}

	return 0;
}