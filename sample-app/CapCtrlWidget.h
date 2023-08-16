#pragma once

#include "imgui/imgui.h"
/* open or close the camera */
#include "Camera.h"

namespace GLUI {
	/* helper to set MatWidget for captured mat */
	void cap_ctrl_widget_Init(void* widget);
	/* draw capture control widget */
	void cap_ctrl_widget_Render();
}