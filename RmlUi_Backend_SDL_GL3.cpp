/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "RmlUi_Backend.h"
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_GL3.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FileInterface.h>
#include <SDL.h>

#if defined RMLUI_PLATFORM_EMSCRIPTEN
	#include <emscripten.h>
#else
	#if !(SDL_VIDEO_RENDER_OGL)
		#error "Only the OpenGL SDL backend is supported."
	#endif
#endif

/**
    Global data used by this backend.

    Lifetime governed by the calls to Backend::Initialize() and Backend::Shutdown().
 */
struct BackendData {
	SystemInterface_SDL system_interface;
	RenderInterface_GL3 render_interface;

	SDL_Window* window = nullptr;
	SDL_GLContext glcontext = nullptr;

	bool running = true;
};
static Rml::UniquePtr<BackendData> data;

bool Backend::Initialize()
{
	RMLUI_ASSERT(!data);

	SDL_Window* window = SDL_GL_GetCurrentWindow();
	if (!window)
		return false;

//#if defined RMLUI_PLATFORM_EMSCRIPTEN
//	// GLES 3.0 (WebGL 2.0)
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
//#else
//	// GL 3.3 Core
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
//#endif
//
//	// Request stencil buffer of at least 8-bit size to supporting clipping on transformed elements.
//	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
//	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
//
//	// Enable linear filtering and MSAA for better-looking visuals, especially when transforms are applied.
//	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
//	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
//	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
	SDL_GLContext glcontext = SDL_GL_GetCurrentContext();

	if (!RmlGL3::Initialize())
	{
		fprintf(stderr, "Could not initialize OpenGL");
		return false;
	}

	data = Rml::MakeUnique<BackendData>();

	if (!data->render_interface)
	{
		data.reset();
		fprintf(stderr, "Could not initialize OpenGL3 render interface.");
		return false;
	}

	data->window = window;
	data->glcontext = glcontext;

	data->system_interface.SetWindow(window);
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	data->render_interface.SetViewport(w, h);
	return true;
}

void Backend::Shutdown()
{
	RMLUI_ASSERT(data);

	SDL_GL_DeleteContext(data->glcontext);
	SDL_DestroyWindow(data->window);

	data.reset();

	SDL_Quit();
}

Rml::SystemInterface* Backend::GetSystemInterface()
{
	RMLUI_ASSERT(data);
	return &data->system_interface;
}

Rml::RenderInterface* Backend::GetRenderInterface()
{
	RMLUI_ASSERT(data);
	return &data->render_interface;
}

bool Backend::ProcessEvents(Rml::Context* context, KeyDownCallback key_down_callback, bool power_save)
{
	RMLUI_ASSERT(data && context);

#if defined RMLUI_PLATFORM_EMSCRIPTEN

	// Ideally we would hand over control of the main loop to emscripten:
	//
	//  // Hand over control of the main loop to the WebAssembly runtime.
	//  emscripten_set_main_loop_arg(EventLoopIteration, (void*)user_data_handle, 0, true);
	//
	// The above is the recommended approach. However, as we don't control the main loop here we have to make due with another approach. Instead, use
	// Asyncify to yield by sleeping.
	// Important: Must be linked with option -sASYNCIFY
	emscripten_sleep(1);

#endif

	bool result = data->running;
	data->running = true;

	SDL_Event ev;
	int has_event = 0;
	if(power_save)
		has_event = SDL_WaitEventTimeout(&ev, static_cast<int>(Rml::Math::Min(context->GetNextUpdateDelay(), 10.0)*1000));
	else has_event = SDL_PollEvent(&ev);
	while (has_event)
	{
		switch (ev.type)
		{
		case SDL_QUIT:
		{
			result = false;
		}
		break;
		case SDL_KEYDOWN:
		{
			const Rml::Input::KeyIdentifier key = RmlSDL::ConvertKey(ev.key.keysym.sym);
			const int key_modifier = RmlSDL::GetKeyModifierState();
			const float native_dp_ratio = 1.f;

			// See if we have any global shortcuts that take priority over the context.
			if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, true))
				break;
			// Otherwise, hand the event over to the context by calling the input handler as normal.
			if (!RmlSDL::InputEventHandler(context, ev))
				break;
			// The key was not consumed by the context either, try keyboard shortcuts of lower priority.
			if (key_down_callback && !key_down_callback(context, key, key_modifier, native_dp_ratio, false))
				break;
		}
		break;
		case SDL_WINDOWEVENT:
		{
			switch (ev.window.event)
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			{
				Rml::Vector2i dimensions(ev.window.data1, ev.window.data2);
				data->render_interface.SetViewport(dimensions.x, dimensions.y);
			}
			break;
			}
			RmlSDL::InputEventHandler(context, ev);
		}
		break;
		default:
		{
			RmlSDL::InputEventHandler(context, ev);
		}
		break;
		}
		has_event = SDL_PollEvent(&ev);
	}

	return result;
}

void Backend::RequestExit()
{
	RMLUI_ASSERT(data);

	data->running = false;
}

void Backend::BeginFrame()
{
	RMLUI_ASSERT(data);

	//data->render_interface.Clear();
	data->render_interface.BeginFrame();
}

void Backend::PresentFrame()
{
	RMLUI_ASSERT(data);

	data->render_interface.EndFrame();
	//SDL_GL_SwapWindow(data->window);
}
