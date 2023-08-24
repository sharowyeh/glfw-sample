#include "HandleWidget.h"
#include "imgui/imgui_internal.h"


// ImVec2 add operator
ImVec2 operator+ (ImVec2 base, const ImVec2& operand) noexcept
{
	return ImVec2(base.x + operand.x, base.y + operand.y);
}
// ImVec2 minus operator
ImVec2 operator- (ImVec2 base, const ImVec2& operand) noexcept
{
	return ImVec2(base.x - operand.x, base.y - operand.y);
}
// ImVec2 add-equal operator
ImVec2& operator+= (ImVec2& base, const ImVec2& operand) noexcept
{
	base.x += operand.x;
	base.y += operand.y;
	return base;
}
// ImVec2 minus-equal operator
ImVec2& operator-= (ImVec2& base, const ImVec2& operand) noexcept
{
	base.x -= operand.x;
	base.y -= operand.y;
	return base;
}
// ImRect add x,y shifted value of ImVec2 to both Min and Max (works as moving the rectangle to bottom-right corner)
ImRect operator+ (ImRect base, const ImVec2& movement) noexcept
{
	// wont change anything to base or let ImRect& base?
	return ImRect(
		base.Min.x + movement.x,
		base.Min.y + movement.y,
		base.Max.x + movement.x,
		base.Max.y + movement.y);
}
// ImRect minux x,y shifted value of ImVec2 to both Min and Max (works as moving the rectangle to top-left corner)
ImRect operator- (ImRect base, const ImVec2& movement) noexcept
{
	// wont change anything to base or let ImRect& base?
	return ImRect(
		base.Min.x - movement.x,
		base.Min.y - movement.y,
		base.Max.x - movement.x,
		base.Max.y - movement.y);
}
// ImRect equal-add x,y shifted value of ImVec2 to both Min and Max (works as moving the rectangle to bottom-right corner)
ImRect& operator+= (ImRect& base, const ImVec2& movement) noexcept
{
	base.Min += movement;
	base.Max += movement;
	return base;
}
// ImRect equal-minux x,y shifted value of ImVec2 to both Min and Max (works as moving the rectangle to top-left corner)
ImRect& operator-= (ImRect& base, const ImVec2& movement) noexcept
{
	base.Min -= movement;
	base.Max -= movement;
	return base;
}

/* should aways be parent pos + content region min */
ImVec2 overlay_offset = ImVec2(0, 0);
/* should aways be parent content region max - min */
ImVec2 overlay_size = ImVec2(0, 0);

void GLUI::BeginHandleLayer()
{
	auto ctx = GImGui;
	auto flags =
		//ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings;
	// always refer to parent window
	auto root = ImGui::GetCurrentWindow();
	auto pos = ImGui::GetWindowPos();
	auto topleft = ImGui::GetWindowContentRegionMin();
	auto bottomright = ImGui::GetWindowContentRegionMax();
	overlay_offset = pos + topleft;
	overlay_size = bottomright - topleft;
	if (root) {
		ImGui::SetNextWindowPos(overlay_offset);
		ImGui::SetNextWindowSize(overlay_size);
	}
	else
		printf("[WARN] handle overlay should be child of a parent window\n");
	//TODO: make public variable?
	bool open = true;
	//ImGui::Begin("##HandleLayer", &open, flags);
	ImGui::BeginChild("##HandleLayer", overlay_size, false, flags);
}

void GLUI::EndHandleLayer()
{
	//ImGui::End();
	ImGui::EndChild();
}

bool GLUI::HandleWidget(float& x, float& y, float& width, float& height, std::string name, bool centered)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	// pointer address as identifier
	const ImGuiID id = window->GetID(name.c_str());
	ImGuiButtonFlags flags = 0; //ImGuiMouseButton_Left

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
	rect = rect + overlay_offset;

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

	auto color_enum = activated ? ImGuiCol_ButtonActive : ImGuiCol_Button;
	color_enum = (hovered || pressed || held) ? ImGuiCol_ButtonHovered : color_enum;
	auto color = ImColor(ImGui::GetStyle().Colors[color_enum]);
	auto draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(rect.Min, rect.Max, color, 2.0f);

	//TODO: make new child objects for corners resizing this HandleWidget,
	//      associate group activation but different mouse button behavior.
	//      have been tried impl here but no good to separate actions

	if (activated == false)
		return true;

	// move rect when left button drag area on the handle widget
	if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		x += ImGui::GetIO().MouseDelta.x;
		y += ImGui::GetIO().MouseDelta.y;
	}

	return true;
}
