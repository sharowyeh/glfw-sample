#include <stdio.h>

//#include "Window.hpp"
#include "MatWidget.h"
#include "Camera.h"

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

#include <thread>
#include <mutex>
#include <condition_variable>

void errorCallback(int code, const char* msg) {
	printf("error:%d msg:%s\n", code, msg);
}

GLUI::MatWidget *cap_widget;


/* cap ctrl UI properties */
bool is_cap_opened = false;
const char* cap_open_text = "Open";
const char* cap_auto_open_text = "Auto open";
bool is_cap_auto_open = false;
const char* cap_show_text = "Hide";
const char* cap_draw_text = "Undraw";

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

/* cap ctrl draw */
void cap_ctrl_widget_Render() {
	ImGui::Begin("Camera Control", NULL, ImGuiWindowFlags_NoCollapse);

	if (ImGui::Button(cap_open_text)) {
		if (is_cap_opened) {
			Camera::CloseCapture();
			is_cap_opened = false;
			cap_open_text = "Open";
		}
		else {
			is_cap_opened = Camera::OpenCapture(0);
			cap_open_text = is_cap_opened ? "Close Camera" : "Failed";
		}
	}
	ImGui::SameLine();
	if (ImGui::Checkbox(cap_auto_open_text, &is_cap_auto_open)) {
		
	}

	if (ImGui::Button(cap_show_text)) {
		cap_widget->IsShow = !cap_widget->IsShow;
		cap_show_text = cap_widget->IsShow ? "Hide" : "Show";
	}
	ImGui::SameLine();
	if (ImGui::Button(cap_draw_text)) {
		cap_widget->IsDraw = !cap_widget->IsDraw;
		cap_draw_text = cap_widget->IsDraw ? "Undraw" : "Draw";
	}
	
	ImGuiIO& io = ImGui::GetIO();
	if (ImGui::TreeNode("Mouse State"))
	{
		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse pos: <INVALID>");
		ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);

		int count = IM_ARRAYSIZE(io.MouseDown);
		ImGui::Text("Mouse down:");         for (int i = 0; i < count; i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
		ImGui::Text("Mouse clicked:");      for (int i = 0; i < count; i++) if (ImGui::IsMouseClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d (%d)", i, ImGui::GetMouseClickedCount(i)); }
		ImGui::Text("Mouse released:");     for (int i = 0; i < count; i++) if (ImGui::IsMouseReleased(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
		ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);
		ImGui::Text("Pen Pressure: %.1f", io.PenPressure); // Note: currently unused
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Keyboard, Gamepad & Navigation State"))
	{
		struct funcs { static bool IsLegacyNativeDupe(ImGuiKey key) { return key < 512 && ImGui::GetIO().KeyMap[key] != -1; } }; // Hide Native<>ImGuiKey duplicates when both exists in the array
		const ImGuiKey key_first = 0;
		//ImGui::Text("Legacy raw:");       for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (io.KeysDown[key]) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
		ImGui::Text("Keys down:");          for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyDown(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d (%.02f secs)", ImGui::GetKeyName(key), key, ImGui::GetKeyData(key)->DownDuration); } }
		ImGui::Text("Keys pressed:");       for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyPressed(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
		ImGui::Text("Keys released:");      for (ImGuiKey key = key_first; key < ImGuiKey_COUNT; key++) { if (funcs::IsLegacyNativeDupe(key)) continue; if (ImGui::IsKeyReleased(key)) { ImGui::SameLine(); ImGui::Text("\"%s\" %d", ImGui::GetKeyName(key), key); } }
		ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
		ImGui::Text("Chars queue:");        for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.
		ImGui::Text("NavInputs down:");     for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f (%.02f secs)", i, io.NavInputs[i], io.NavInputsDownDuration[i]); }
		ImGui::Text("NavInputs pressed:");  for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { ImGui::SameLine(); ImGui::Text("[%d]", i); }

		// Draw an arbitrary US keyboard layout to visualize translated keys
		{
			const ImVec2 key_size = ImVec2(35.0f, 35.0f);
			const float  key_rounding = 3.0f;
			const ImVec2 key_face_size = ImVec2(25.0f, 25.0f);
			const ImVec2 key_face_pos = ImVec2(5.0f, 3.0f);
			const float  key_face_rounding = 2.0f;
			const ImVec2 key_label_pos = ImVec2(7.0f, 4.0f);
			const ImVec2 key_step = ImVec2(key_size.x - 1.0f, key_size.y - 1.0f);
			const float  key_row_offset = 9.0f;

			ImVec2 board_min = ImGui::GetCursorScreenPos();
			ImVec2 board_max = ImVec2(board_min.x + 3 * key_step.x + 2 * key_row_offset + 10.0f, board_min.y + 3 * key_step.y + 10.0f);
			ImVec2 start_pos = ImVec2(board_min.x + 5.0f - key_step.x, board_min.y);

			struct KeyLayoutData { int Row, Col; const char* Label; ImGuiKey Key; };
			const KeyLayoutData keys_to_display[] =
			{
				{ 0, 0, "", ImGuiKey_Tab },      { 0, 1, "Q", ImGuiKey_Q }, { 0, 2, "W", ImGuiKey_W }, { 0, 3, "E", ImGuiKey_E }, { 0, 4, "R", ImGuiKey_R },
				{ 1, 0, "", ImGuiKey_CapsLock }, { 1, 1, "A", ImGuiKey_A }, { 1, 2, "S", ImGuiKey_S }, { 1, 3, "D", ImGuiKey_D }, { 1, 4, "F", ImGuiKey_F },
				{ 2, 0, "", ImGuiKey_LeftShift },{ 2, 1, "Z", ImGuiKey_Z }, { 2, 2, "X", ImGuiKey_X }, { 2, 3, "C", ImGuiKey_C }, { 2, 4, "V", ImGuiKey_V }
			};

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			draw_list->PushClipRect(board_min, board_max, true);
			for (int n = 0; n < IM_ARRAYSIZE(keys_to_display); n++)
			{
				const KeyLayoutData* key_data = &keys_to_display[n];
				ImVec2 key_min = ImVec2(start_pos.x + key_data->Col * key_step.x + key_data->Row * key_row_offset, start_pos.y + key_data->Row * key_step.y);
				ImVec2 key_max = ImVec2(key_min.x + key_size.x, key_min.y + key_size.y);
				draw_list->AddRectFilled(key_min, key_max, IM_COL32(204, 204, 204, 255), key_rounding);
				draw_list->AddRect(key_min, key_max, IM_COL32(24, 24, 24, 255), key_rounding);
				ImVec2 face_min = ImVec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
				ImVec2 face_max = ImVec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
				draw_list->AddRect(face_min, face_max, IM_COL32(193, 193, 193, 255), key_face_rounding, ImDrawFlags_None, 2.0f);
				draw_list->AddRectFilled(face_min, face_max, IM_COL32(252, 252, 252, 255), key_face_rounding);
				ImVec2 label_min = ImVec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
				draw_list->AddText(label_min, IM_COL32(64, 64, 64, 255), key_data->Label);
				if (ImGui::IsKeyDown(key_data->Key))
					draw_list->AddRectFilled(key_min, key_max, IM_COL32(255, 0, 0, 128), key_rounding);
			}
			draw_list->PopClipRect();
			ImGui::Dummy(ImVec2(board_max.x - board_min.x, board_max.y - board_min.y));
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Capture override"))
	{
		ImGui::Button("Hovering me sets the\nkeyboard capture flag");
		if (ImGui::IsItemHovered())
			ImGui::CaptureKeyboardFromApp(true);
		ImGui::SameLine();
		ImGui::Button("Holding me clears the\nthe keyboard capture flag");
		if (ImGui::IsItemActive())
			ImGui::CaptureKeyboardFromApp(false);
		ImGui::TreePop();
	}


	ImGui::Text("average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();
}

int main() {
	GLFWwindow* window;
	
	glfwSetErrorCallback(errorCallback);

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


	cap_widget = new GLUI::MatWidget("Capture", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	Camera::SetRawWidget(cap_widget);

	/* loop until user close window */
	while (glfwWindowShouldClose(window) == 0) {

		/* poll for and process event */
		glfwPollEvents();
		/* render after clear */
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (cap_widget->IsShow) {
			cap_widget->Render();
		}

		ImGui::SetNextWindowPos(ImVec2(670, 0));
		cap_ctrl_widget_Render();

        //ImGui::ShowDemoWindow();

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