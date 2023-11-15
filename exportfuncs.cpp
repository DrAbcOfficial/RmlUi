#include <metahook.h>
#include <studio.h>
#include <r_studioint.h>
#include "cl_entity.h"
#include "com_model.h"
#include "triangleapi.h"
#include "cvardef.h"
#include "exportfuncs.h"
#include "entity_types.h"
#include <RmlUi/Core.h>
#include "RmlUi_Backend.h"
#include <RmlUi/Debugger/Debugger.h>

cl_enginefunc_t gEngfuncs;
engine_studio_api_t IEngineStudio;
r_studio_interface_t** gpStudioInterface;

Rml::Context* g_pContext;
void HUD_Init() {
	gExportfuncs.HUD_Init();
	SCREENINFO_s si;
	si.iSize = sizeof(si);
	gEngfuncs.pfnGetScreenInfo(&si);
	// Create a context to display documents within.
	g_pContext = Rml::CreateContext("main", Rml::Vector2i(si.iWidth, si.iHeight));
	Rml::Debugger::Initialise(g_pContext);
	Rml::ElementDocument* document = g_pContext->LoadDocument("rmltest/demo.rml");
	if (document)
		document->Show();
}

void HUD_Frame(double a1){
	gExportfuncs.HUD_Frame(a1);
	g_pContext->Update();
	Backend::BeginFrame();
	g_pContext->Render();
	Backend::PresentFrame();
}