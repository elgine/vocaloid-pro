#pragma once
#include "ane.h"
extern "C" {

	void ExtensionInitializer(void** extDataToSet,
							FREContextInitializer* ctxInitializerToSet, 
							FREContextFinalizer* ctxFinalizerToSet);
	
	void ExtensionFinalizer(void* extData);
}