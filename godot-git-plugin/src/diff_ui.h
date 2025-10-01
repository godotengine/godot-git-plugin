#pragma once

#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/rich_text_label.hpp"
#include "godot_cpp/classes/option_button.hpp"
#include "godot_cpp/classes/editor_vcs_interface.hpp"
#include "godot_cpp/templates/list.hpp"

namespace godot {

struct DiffUI {
	VBoxContainer *diff_dock = nullptr;
	Button *version_control_dock_button = nullptr;
	Label *diff_title = nullptr;
	RichTextLabel *diff = nullptr;
	OptionButton *diff_view_type_select = nullptr;
	bool show_commit_diff_header = false;
	List<EditorVCSInterface::DiffFile> diff_content;
};

} // namespace godot
