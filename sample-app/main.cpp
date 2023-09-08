#include <stdio.h>

#include "Window.hpp"
#include "MatWidget.h"
#include "Camera.h"
#include "CapCtrlWidget.h"
#include "TimeoutPopup.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#ifdef _DEBUG
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glfw3.lib")
#else
#pragma comment(lib, "glfw3_mt.lib")
#endif
#pragma comment(lib, "opengl32.lib")

#include <opencv2/opencv.hpp>
#ifdef _DEBUG
#pragma comment(lib, "opencv_world455d.lib")
#else
#pragma comment(lib, "opencv_world455.lib")
#endif

GLUI::Window* window;
GLUI::MatWidget *cap_widget;
GLUI::TimeoutPopup* leave_popup;

static void CtrlStatusHandler_ClearAll(ImGuiContext* ctx, ImGuiSettingsHandler*) {
	printf("ctrl status: clear all\n");
}

static void* CtrlStatusHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name) {
	printf("ctrl status: read open\n");
	return NULL;
}

static void CtrlStatusHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line) {
	printf("ctrl status: read line\n");
}

static void CtrlStatusHandler_ApplyAll(ImGuiContext* ctx, ImGuiSettingsHandler*) {
	printf("ctrl status: apply all\n");
}

static void CtrlStatusHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
	printf("ctrl status: write all\n");
}

/* ini handlers to save control status
refer to ImGui::Initialize() or ImGui::TableSettingsAddSettingsHandler() */
void custom_settings_ini_handler() {
	ImGuiSettingsHandler ini_handler;
	ini_handler.TypeName = "ControlStatus";
	ini_handler.TypeHash = ImHashStr("ControlStatus");
	ini_handler.ClearAllFn = CtrlStatusHandler_ClearAll;
	ini_handler.ReadOpenFn = CtrlStatusHandler_ReadOpen;
	ini_handler.ReadLineFn = CtrlStatusHandler_ReadLine;
	ini_handler.ApplyAllFn = CtrlStatusHandler_ApplyAll;
	ini_handler.WriteAllFn = CtrlStatusHandler_WriteAll;
	ImGui::AddSettingsHandler(&ini_handler);
}

/* draggable main,
*  NOTE: use get mouse drag delta instead of mouse delta,
*    prevent mouse pos be calculated by moved main window to the next rendering loop results opposite values of previous
*/
void main_draggable_loop(GLFWwindow *window) {
	if (ImGui::IsAnyItemActive()) {
		return;
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) == false) {
		return;
	}
	int x, y;
	float dx, dy;
	glfwGetWindowPos(window, &x, &y);
	auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
	dx = delta.x;
	dy = delta.y;
	GLUI::update_mainwindow_info(x, y, dx, dy);
	x += floor(dx);
	y += floor(dy);
	glfwSetWindowPos(window, x, y);
}

int main() {

	window = GLUI::Window::Create("GLFW", 1280, 720);
	window->SetWindowOpacity(0.8f);
	window->OnWindowClosing = [](GLUI::Window* wnd) {
		// this callback not design for rendering loops, all things can do is releasing resources before window closing
		printf("on windoe close callback\n");
		if (leave_popup) {
			// prevent main window closing from default behavior, hand over to popup callback -> timeout slapsed
			glfwSetWindowShouldClose(window->GetGlfwWindow(), GLFW_FALSE);
			// raise timeout popup displaying time remaining
			leave_popup->Show(true, 3.f);
		}
	};
	window->OnRenderFrame = [](GLUI::Window* wnd) {
		// is the same afterward the window->PrepareFrame()
	};
	
	// apply custom callbacks to settings handler
	custom_settings_ini_handler();

	cap_widget = new GLUI::MatWidget("Capture", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	Camera::SetRawWidget(cap_widget);
	// assign for capture controls UI
	GLUI::cap_ctrl_widget_Init();

	// assign close window behavior
	leave_popup = new GLUI::TimeoutPopup(window->GetGlfwWindow());
	leave_popup->OnTimeoutElapsed = [](GLFWwindow* glWnd) {
		printf("on timeout elapsed\n");
		// exit message loop to close main window
		glfwSetWindowShouldClose(window->GetGlfwWindow(), GLFW_TRUE);
	};
	cap_ctrl_set_close_popup(leave_popup);

	/* loop until user close window */
	while (window->PrepareFrame() == 0) {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f); // must pair with PopStyleVar() restore style changes for rendering loop

		if (cap_widget->IsShow) {
			cap_widget->Render();
		}

		ImGui::SetNextWindowPos(ImVec2(670, 0));
		GLUI::cap_ctrl_widget_Render(cap_widget);

        //ImGui::ShowDemoWindow();
		
		/* enable draggable window from context */
		main_draggable_loop(window->GetGlfwWindow());

		leave_popup->Render();
		
		ImGui::PopStyleVar();

		window->SwapWindow();
	}

	Camera::CloseCapture();

	window->Destroy();

	return 0;
}