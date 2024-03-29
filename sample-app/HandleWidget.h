#pragma once
/*
* designed mouse draggable GUI overlaying on caller widget
*/

#include <string>
#include "imgui/imgui.h"

// from https://github.com/ocornut/imgui/issues/553
// for simple overlay drag and drop objects

namespace GLUI {

	// will create a window to place handle widgets
	void BeginHandleLayer();

	void EndHandleLayer();

	// NOTE: can call directly to overlay on located window, or use BeginHandleLayer() to create new window
	// given point, size and given point is centered of size
	bool HandleWidget(float& x, float& y, float& width, float& height, std::string name, bool centered = false);

}