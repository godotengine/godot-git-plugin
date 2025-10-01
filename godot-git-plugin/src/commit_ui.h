#pragma once

#include "godot_cpp/classes/button.hpp"
#include "godot_cpp/classes/h_box_container.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/v_box_container.hpp"
#include "godot_cpp/classes/popup_menu.hpp"
#include "godot_cpp/classes/confirmation_dialog.hpp"
#include "godot_cpp/classes/option_button.hpp"
#include "godot_cpp/classes/accept_dialog.hpp"
#include "godot_cpp/classes/check_button.hpp"
#include "godot_cpp/classes/line_edit.hpp"
#include "godot_cpp/classes/file_dialog.hpp"
#include "godot_cpp/classes/editor_vcs_interface.hpp"
#include "godot_cpp/classes/tree.hpp"
#include "godot_cpp/classes/menu_button.hpp"
#include "godot_cpp/classes/text_edit.hpp"
#include "godot_cpp/classes/separator.hpp"
#include "godot_cpp/templates/hash_map.hpp"

namespace godot {

struct CommitUI {
	VBoxContainer *commit_dock = nullptr;

	AcceptDialog *discard_all_confirm = nullptr;

	OptionButton *commit_list_size_button = nullptr;

	AcceptDialog *branch_create_confirm = nullptr;
	LineEdit *branch_create_name_input = nullptr;
	Button *branch_create_ok = nullptr;

	AcceptDialog *remote_create_confirm = nullptr;
	LineEdit *remote_create_name_input = nullptr;
	LineEdit *remote_create_url_input = nullptr;
	Button *remote_create_ok = nullptr;

	HashMap<EditorVCSInterface::ChangeType, String> change_type_to_strings;
	HashMap<EditorVCSInterface::ChangeType, Color> change_type_to_color;
	HashMap<EditorVCSInterface::ChangeType, Ref<Texture>> change_type_to_icon;

	Tree *staged_files = nullptr;
	Tree *unstaged_files = nullptr;
	Tree *commit_list = nullptr;

	OptionButton *branch_select = nullptr;
	Button *branch_remove_button = nullptr;
	AcceptDialog *branch_remove_confirm = nullptr;

	Button *fetch_button = nullptr;
	Button *pull_button = nullptr;
	Button *push_button = nullptr;
	OptionButton *remote_select = nullptr;
	Button *remote_remove_button = nullptr;
	AcceptDialog *remote_remove_confirm = nullptr;
	MenuButton *extra_options = nullptr;
	PopupMenu *extra_options_remove_branch_list = nullptr;
	PopupMenu *extra_options_remove_remote_list = nullptr;

	String branch_to_remove;
	String remote_to_remove;

	Button *stage_all_button = nullptr;
	Button *unstage_all_button = nullptr;
	Button *discard_all_button = nullptr;
	Button *refresh_button = nullptr;
	TextEdit *commit_message = nullptr;
	Button *commit_button = nullptr;

	CommitUI();
	~CommitUI();
};
} // namespace godot
