#include "ane_entry.h"
#include "ane_helper.hpp"
#include "interface.h"

FREObject SetExtractTempPath(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {
	
}

FREObject DisposeTemp(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject SetPreviewEffect(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject SetPreviewLoop(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject SetPreviewSegments(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject OpenPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject SetPreviewEffectOptions(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject StartPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject StopPreview(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject Seek(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject Render(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject StopRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject StopRenderAll(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject CancelRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject CancelRenderAll(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

FREObject ForceExitRender(FREContext ctx, void* funcData, uint32_t argc, FREObject argv[]) {

}

void ContextInitializer(
	void					 * extData,
	const uint8_t			 * ctxType,
	FREContext				   ctx,
	uint32_t				 * numFunctionsToSet,
	const FRENamedFunction	** functionsToSet){

	static FRENamedFunction extension_functions[] = {
		{ (const uint8_t*) "setExtractTempPath", NULL, &SetExtractTempPath },
		{ (const uint8_t*) "disposeTemp", NULL, &DisposeTemp },
		{ (const uint8_t*) "setPreviewEffect", NULL, &SetPreviewEffect },
		{ (const uint8_t*) "setPreviewLoop", NULL, &SetPreviewLoop },
		{ (const uint8_t*) "setPreviewSegments", NULL, &SetPreviewSegments },
		{ (const uint8_t*) "openPreview", NULL, &OpenPreview },
		{ (const uint8_t*) "setPreviewEffectOptions", NULL, &SetPreviewEffectOptions },
		{ (const uint8_t*) "startPreview", NULL, &StartPreview },
		{ (const uint8_t*) "stopPreview", NULL, &StopPreview },
		{ (const uint8_t*) "seek", NULL, &Seek },
		{ (const uint8_t*) "render", NULL, &Render },
		{ (const uint8_t*) "stopRender", NULL, &StopRender },
		{ (const uint8_t*) "stopRenderAll", NULL, &StopRenderAll },
		{ (const uint8_t*) "cancelRender", NULL, &CancelRender },
		{ (const uint8_t*) "cancelRenderAll", NULL, &CancelRenderAll },
		{ (const uint8_t*) "forceExitRender", NULL, &ForceExitRender }
	};

	*numFunctionsToSet = 16;
	*functionsToSet = extension_functions;
}

void ContextFinalizer(FREContext ctx){
	return;
}


void ExtensionInitializer(void** extDataToSet,
	FREContextInitializer* ctxInitializerToSet,
	FREContextFinalizer* ctxFinalizerToSet) {
	*ctxInitializerToSet = &ContextInitializer;
	*ctxFinalizerToSet = &ContextFinalizer;
}

void ExtensionFinalizer(void* extData) {}