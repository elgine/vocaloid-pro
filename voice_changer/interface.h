#pragma once
#include "ane.h"

//void contextInitializer(
//	void					 * extData,
//	const uint8_t			 * ctxType,
//	FREContext				   ctx,
//	uint32_t				 * numFunctionsToSet,
//	const FRENamedFunction	** functionsToSet)
//{
//	// Create mapping between function names and pointers in an array of FRENamedFunction.
//	// These are the functions that you will call from ActionScript -
//	// effectively the interface of your native library.
//	// Each member of the array contains the following information:
//	// { function name as it will be called from ActionScript,
//	//   any data that should be passed to the function,
//	//   a pointer to the implementation of the function in the native library }
//	static FRENamedFunction extensionFunctions[] =
//	{
//		{ (const uint8_t*) "displayToast", NULL, &displayToast }
//	};

//	// Tell AIR how many functions there are in the array:
//	*numFunctionsToSet = sizeof(extensionFunctions) / sizeof(FRENamedFunction);

//	// Set the output parameter to point to the array we filled in:
//	*functionsToSet = extensionFunctions;
//}

//void contextFinalizer(FREContext ctx)
//{
//	return;
//}

extern "C" {

	//__declspec(dllexport) void ExtensionInitializer(void** extData, FREContextInitializer* ctxInitializer, FREContextFinalizer* ctxFinalizer)
	//{
	//	*ctxInitializer = &contextInitializer; // The name of function that will intialize the extension context
	//	*ctxFinalizer = &contextFinalizer; // The name of function that will finalize the extension context
	//}

	//__declspec(dllexport) void ExtensionFinalizer(void* extData)
	//{
	//	return;
	//}

	void SetExtractTempPath(const char* path);

	void DisposeTemp();
	
	int SetPreviewEffect(int id);

	void SetPreviewLoop(bool v);

	int SetPreviewSegments(int64_t**, int count);

	int OpenPreview(const char* path);

	void SetPreviewEffectOptions(float* options, int count);

	int StartPreview();

	int StopPreview();

	void Render(const char** sources, const char **dest, int *effect_ids,
				float **options, int *option_count, int count,
				int64_t*** segmentses = nullptr, int *segment_counts = nullptr);

	void StopRender(const char* source);

	void StopRenderAll();

	void CancelRender(const char* source);

	void CancelRenderAll();

	void ForceExitRender();
}