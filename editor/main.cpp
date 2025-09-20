
#include <vector>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include <SDL3/SDL.h>
#include <plush/plush.h>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>

static SDL_AppResult die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, fmt, ap);
	va_end(ap);

	SDL_Event event;
	event.quit.type = SDL_EVENT_QUIT;
	event.quit.timestamp = SDL_GetTicksNS();
	SDL_PushEvent(&event);

	return SDL_APP_FAILURE;
}

class app_t {
public:
	void EditorMain()
	{
		ImGui::ShowDemoWindow(nullptr);
	}

	SDL_AppResult Iterate()
	{
		ImGui_ImplSDLRenderer3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		EditorMain();

		ImGui::Render();
		ImGuiIO &io = ImGui::GetIO();
		SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
		SDL_RenderPresent(renderer);

		return SDL_APP_CONTINUE;
	}

	SDL_AppResult Event(SDL_Event *event)
	{
		if ((event->type == SDL_EVENT_QUIT) || (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event->window.windowID == SDL_GetWindowID(window)))
			return SDL_APP_SUCCESS;

		ImGui_ImplSDL3_ProcessEvent(event);

		return SDL_APP_CONTINUE;
	}

	app_t(const char *title, int w=1280, int h=720) : windowTitle(title), windowWidth(w), windowHeight(h)
	{
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
			throw std::runtime_error(SDL_GetError());

		if (!SDL_CreateWindowAndRenderer(windowTitle.c_str(), windowWidth, windowHeight, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_MAXIMIZED, &window, &renderer))
			throw std::runtime_error(SDL_GetError());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO &io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.IniFilename = nullptr;
		io.LogFilename = nullptr;

		ImGui::StyleColorsDark();

		ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
		ImGui_ImplSDLRenderer3_Init(renderer);

		SDL_ShowWindow(window);
	}

	~app_t()
	{
		ImGui_ImplSDLRenderer3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();

		if (renderer) SDL_DestroyRenderer(renderer);
		if (window) SDL_DestroyWindow(window);

		SDL_Quit();
	}
private:
	// SDL video state
	SDL_Window *window;
	SDL_Renderer *renderer;

	int windowWidth;
	int windowHeight;
	std::string windowTitle;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
};

SDLMAIN_DECLSPEC SDL_AppResult SDLCALL SDL_AppInit(void **appstate, SDL_UNUSED int argc, SDL_UNUSED char *argv[])
{
	app_t *app = NULL;

	try {
		app = new app_t("Plush Scene Editor");
	} catch (const std::runtime_error &error) {
		return die("%s", error.what());
	}

	*appstate = reinterpret_cast<void*>(app);

	return SDL_APP_CONTINUE;
}

SDLMAIN_DECLSPEC SDL_AppResult SDLCALL SDL_AppIterate(void *appstate)
{
	app_t *app = reinterpret_cast<app_t*>(appstate);

	return app->Iterate();
}

SDLMAIN_DECLSPEC SDL_AppResult SDLCALL SDL_AppEvent(void *appstate, SDL_Event *event)
{
	app_t *app = reinterpret_cast<app_t*>(appstate);

	return app->Event(event);
}

SDLMAIN_DECLSPEC void SDLCALL SDL_AppQuit(void *appstate, SDL_UNUSED SDL_AppResult result)
{
	app_t *app = reinterpret_cast<app_t*>(appstate);

	delete app;
}
