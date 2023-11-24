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
	GLFWwindow* window;

	glfwSetErrorCallback([](int code, const char* msg) {
		printf("error:%d msg:%s\n", code, msg);
	});

	/* init framework */
	if (!glfwInit())
		return -1;

	/* remove window caption */
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	/* topmost */
	//glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

	window = glfwCreateWindow(1280, 720, "GLFW", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwSetWindowOpacity(window, 0.8f);
	auto close_callback = [](GLFWwindow* wnd) {
		// this callback not design for rendering loops, all things can do is releasing resources before window closing
		printf("on windoe close callback\n");
	};
	glfwSetWindowCloseCallback(window, close_callback);

	/* make context current */
	glfwMakeContextCurrent(window);
	/* for vsync */
	glfwSwapInterval(1);
	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	/* CreateContext -> Initialize -> default ini handlers */
	// apply custom callbacks to settings handler
	custom_settings_ini_handler();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;
	// just like my vs2022
	io.Fonts->AddFontFromFileTTF("CascadiaMono.ttf", 20.f);
	io.FontDefault = io.Fonts->Fonts.back();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");


	cap_widget = new GLUI::MatWidget("Capture", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	Camera::SetRawWidget(cap_widget);
	// assign for capture controls UI
	GLUI::cap_ctrl_widget_Init();

	// assign close window behavior
	leave_popup = new GLUI::TimeoutPopup(window);
	leave_popup->OnTimeoutElasped = close_callback;
	cap_ctrl_set_close_popup(leave_popup);

	/* loop until user close window */
	while (glfwWindowShouldClose(window) == 0) {

		/* poll for and process event */
		glfwPollEvents();
		/* render after clear */
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClear(GL_COLOR_BUFFER_BIT);

		// TODO: mac error can't get backend data from opengl3
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f); // must pair with PopStyleVar() restore style changes for rendering loop

		if (cap_widget->IsShow) {
			cap_widget->Render();
		}

		ImGui::SetNextWindowPos(ImVec2(670, 0));
		GLUI::cap_ctrl_widget_Render(cap_widget);

        //ImGui::ShowDemoWindow();
		
		/* enable draggable window from context */
		main_draggable_loop(window);

		if (glfwWindowShouldClose(window)) {
			// raise timeout popup displaying time remaining
			leave_popup->Show(true, 3.f);
			// reset main window close flag, let main window close popup decide
			glfwSetWindowShouldClose(window, GLFW_FALSE);
		}
		leave_popup->Render();
		
		ImGui::PopStyleVar();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/* swap buffer */
		glfwSwapBuffers(window);
	}

	Camera::CloseCapture();

	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}