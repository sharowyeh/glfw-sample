#include "HandleWidget.h"

void GLUI::BeginHandleLayer()
{
	auto ctx = GImGui;
	auto flags =
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings;
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	if (ctx != NULL)
		ImGui::SetNextWindowSize(ctx->IO.DisplaySize);
	else
		printf("[WARN] ctx is empty! should assign size manually\n");
	//TODO: make public variable?
	bool open = true;
	ImGui::Begin("##HandleLayer", &open, flags);
}

void GLUI::EndHandleLayer()
{
	ImGui::End();
}

bool GLUI::HandleWidget(float& x, float& y, float& width, float& height, std::string name, bool centered)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	// pointer address as identifier
	const ImGuiID id = window->GetID(name.c_str());
	ImGuiButtonFlags flags = 0;

	ImRect rect;
	if (centered) {
		rect = ImRect(
			x - width / 2,
			y - height / 2,
			x + width / 2,
			y + height / 2);
	}
	else {
		rect = ImRect(x, y, x + width, y + height);
	}

	auto ctx = GImGui;
	if (ctx != NULL) {
		auto style = ctx->Style;
		ImGui::ItemSize(rect, style.FramePadding.y);
	}
	else {
		ImGui::ItemSize(rect, 0);
	}
	if (!ImGui::ItemAdd(rect, id))
		return false;

	bool hovered, held, pressed, activated;
	pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, flags);
	activated = ImGui::IsItemActive();

	auto color_enum = (hovered || activated || pressed || held) ? ImGuiCol_ButtonHovered : ImGuiCol_Button;
	auto color = ImColor(ImGui::GetStyle().Colors[color_enum]);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(rect.Min, rect.Max, color, 4.0f);

	if (activated && ImGui::IsMouseDragging(0)) {
		x += ImGui::GetIO().MouseDelta.x;
		y += ImGui::GetIO().MouseDelta.y;
	}

	return true;
}
