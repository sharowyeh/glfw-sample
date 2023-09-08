#pragma once

#include "imgui/imgui.h"
/* open or close the camera */
#include "Camera.h"
#include "MatWidget.h"
// for close window button
#include "TimeoutPopup.h"

namespace GLUI {
	/* helper to set MatWidget for captured mat */
	void cap_ctrl_widget_Init();

	void cap_ctrl_set_close_popup(TimeoutPopup* popup);

	/* draw capture control widget */
	void cap_ctrl_widget_Render(MatWidget* widget = nullptr);

	void update_mainwindow_info(int x, int y, float dx, float dy);

}