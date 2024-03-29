#include "CapCtrlWidget.h"

namespace GLUI {
//TODO: make class?

	/* cap ctrl UI properties */
	bool is_cap_opened = false;
	const char* cap_open_text = "Open";
	const char* cap_auto_open_text = "Auto open";
	bool is_cap_auto_open = false;
	const char* cap_show_text = "Hide";
	const char* cap_draw_text = "Undraw";

	int main_x = 0;
	int main_y = 0;
	float mouse_dx = 0;
	float mouse_dy = 0;

	// for close window button
	TimeoutPopup* leave = nullptr;

	void cap_ctrl_widget_Init() { /* so far nothing to do */ }

	void cap_ctrl_set_close_popup(TimeoutPopup* popup) { leave = popup; }

	/* cap ctrl draw */
	void cap_ctrl_widget_Render(MatWidget* cap_widget) {
		//ImGui::GetStyle().WindowRounding = 4.f; // change style to the next control
		ImGui::Begin("Camera Control", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

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
			if (!cap_widget) {
				cap_show_text = "Unavailable";
			}
			else {
				cap_widget->IsShow = !cap_widget->IsShow;
				cap_show_text = cap_widget->IsShow ? "Hide" : "Show";
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(cap_draw_text)) {
			if (!cap_widget) {
				cap_show_text = "Unavailable";
			}
			else {
				cap_widget->IsDraw = !cap_widget->IsDraw;
				cap_draw_text = cap_widget->IsDraw ? "Undraw" : "Draw";
			}
		}

		// About main window controls - incase window settings changed

		// alter close button if main window caption is hidden
		if (ImGui::Button("Close window")) {
			if (leave) {
				leave->Show(true, 3.f);
			}
			else {
				printf("need to assign timeout popup to display\n");
			}
		}

		ImGui::Text("Main x: %d, y: %d Mouse dx: %.2f, dy: %.2f", main_x, main_y, mouse_dx, mouse_dy);

		// IO events from window

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
			ImGui::Text("Mouse dragging:");     for (int i = 0; i < count; i++) if (ImGui::IsMouseDragging(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);
			ImGui::Text("Pen Pressure: %.1f", io.PenPressure); // Note: currently unused
			ImGui::Text("Item Active: %s", (ImGui::IsItemActive() ? "yes" : "no")); // if last item id was set and eq to active id
			ImGui::Text("Any Item Active: %s, %x", (ImGui::IsAnyItemActive() ? "yes" : "no"), ImGui::GetActiveID());
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

	void update_mainwindow_info(int x, int y, float dx, float dy) {
		main_x = x;
		main_y = y;
		mouse_dx = dx;
		mouse_dy = dy;
	}

} // namespace GLUI