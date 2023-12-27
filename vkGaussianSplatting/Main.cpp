#include "pch.h"
#include <stdexcept>
#include "Engine/Engine.h"

#include "Scenes/SimpleScene.h"
#include "Scenes/SponzaScene.h"
#include "Scenes/DragonScene.h"
#include "Scenes/ManyMeshesScene.h"

int main()
{
	// Set flags for tracking CPU memory leaks
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	// Create engine within it's own scope
	{
		Engine engine;
		engine.init(new SponzaScene());
	}

	// Display validation errors right after exit
#ifdef _DEBUG
	std::cout << "Validation errors are displayed here. Press enter to continue..." << std::endl;
	getchar();
#endif

	return EXIT_SUCCESS;
}