#include <stdio.h>

//#include "Window.hpp"
#include "MatWidget.h"
#include "Camera.h"
#include "CapCtrlWidget.h"
#include "ExitPopup.h"

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
void ctrl_status_add_settings_handler() {
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

/* draggable main, TODO: still buggy, aware dpi scaling, mouse pos diff */
void main_draggable_loop(GLFWwindow *window) {
	if (ImGui::IsAnyItemActive()) {
		return;
	}
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) == false) {
		return;
	}
	int x, y;
	glfwGetWindowPos(window, &x, &y);
	x += ImGui::GetIO().MouseDelta.x;
	y += ImGui::GetIO().MouseDelta.y;
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
	glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);

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
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	ctrl_status_add_settings_handler();


	cap_widget = new GLUI::MatWidget("Capture", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	Camera::SetRawWidget(cap_widget);
	// assign for capture controls UI
	GLUI::cap_ctrl_widget_Init(cap_widget);
	// assign for exiting behavior
	GLUI::exit_popup_Init(window, (GLFWwindowclosefun)close_callback);

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

		if (cap_widget->IsShow) {
			cap_widget->Render();
		}

		ImGui::SetNextWindowPos(ImVec2(670, 0));
		GLUI::cap_ctrl_widget_Render();

        //ImGui::ShowDemoWindow();
		
		main_draggable_loop(window);

		if (glfwWindowShouldClose(window)) {
			GLUI::exit_popup_Update(true);
			// reset main window close flag, let main window close popup decide
			glfwSetWindowShouldClose(window, GLFW_FALSE);
		}
		GLUI::exit_popup_Render();

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