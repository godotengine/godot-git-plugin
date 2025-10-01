#include "commit_ui.h"

namespace godot {

CommitUI::CommitUI() {
	commit_dock = memnew(godot::VBoxContainer);
	commit_dock->set_visible(false);
	commit_dock->set_name("Commit");

	commit_dock = memnew(VBoxContainer);
	commit_dock->set_visible(false);
	commit_dock->set_name("Commit");

	VBoxContainer *unstage_area = memnew(VBoxContainer);
	unstage_area->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	unstage_area->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	commit_dock->add_child(unstage_area);

	HBoxContainer *unstage_title = memnew(HBoxContainer);
	unstage_area->add_child(unstage_title);

	Label *unstage_label = memnew(Label);
	unstage_label->set_text(TTR("Unstaged Changes"));
	unstage_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	unstage_title->add_child(unstage_label);

	refresh_button = memnew(Button);
	refresh_button->set_tooltip_text(TTR("Detect new changes"));
	refresh_button->set_flat(true);
	// refresh_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Reload"), SNAME("EditorIcons")));
	refresh_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_refresh_stage_area));
	refresh_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_refresh_commit_list));
	refresh_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_refresh_branch_list));
	refresh_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_refresh_remote_list));
	unstage_title->add_child(refresh_button);

	discard_all_confirm = memnew(AcceptDialog);
	discard_all_confirm->set_title(TTR("Discard all changes"));
	discard_all_confirm->set_min_size(Size2i(400, 50));
	discard_all_confirm->set_text(TTR("This operation is IRREVERSIBLE. Your changes will be deleted FOREVER."));
	discard_all_confirm->set_hide_on_ok(true);
	discard_all_confirm->set_ok_button_text(TTR("Permanentally delete my changes"));
	discard_all_confirm->add_cancel_button();
	commit_dock->add_child(discard_all_confirm);

	discard_all_confirm->get_ok_button()->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_discard_all));

	discard_all_button = memnew(Button);
	discard_all_button->set_tooltip_text(TTR("Discard all changes"));
	// discard_all_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Close"), SNAME("EditorIcons")));
	discard_all_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_confirm_discard_all));
	discard_all_button->set_flat(true);
	unstage_title->add_child(discard_all_button);

	stage_all_button = memnew(Button);
	stage_all_button->set_flat(true);
	// stage_all_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("MoveDown"), SNAME("EditorIcons")));
	stage_all_button->set_tooltip_text(TTR("Stage all changes"));
	unstage_title->add_child(stage_all_button);

	unstaged_files = memnew(Tree);
	unstaged_files->set_h_size_flags(Tree::SIZE_EXPAND_FILL);
	unstaged_files->set_v_size_flags(Tree::SIZE_EXPAND_FILL);
	unstaged_files->set_select_mode(Tree::SELECT_ROW);
	unstaged_files->connect(SNAME("item_selected"), callable_mp(this, &VersionControlEditorPlugin::_load_diff).bind(unstaged_files));
	unstaged_files->connect(SNAME("item_activated"), callable_mp(this, &VersionControlEditorPlugin::_item_activated).bind(unstaged_files));
	unstaged_files->connect(SNAME("button_clicked"), callable_mp(this, &VersionControlEditorPlugin::_cell_button_pressed));
	unstaged_files->create_item();
	unstaged_files->set_hide_root(true);
	unstage_area->add_child(unstaged_files);

	VBoxContainer *stage_area = memnew(VBoxContainer);
	stage_area->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	stage_area->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	commit_dock->add_child(stage_area);

	HBoxContainer *stage_title = memnew(HBoxContainer);
	stage_area->add_child(stage_title);

	Label *stage_label = memnew(Label);
	stage_label->set_text(TTR("Staged Changes"));
	stage_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	stage_title->add_child(stage_label);

	unstage_all_button = memnew(Button);
	unstage_all_button->set_flat(true);
	unstage_all_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("MoveUp"), SNAME("EditorIcons")));
	unstage_all_button->set_tooltip_text(TTR("Unstage all changes"));
	stage_title->add_child(unstage_all_button);

	staged_files = memnew(Tree);
	staged_files->set_h_size_flags(Tree::SIZE_EXPAND_FILL);
	staged_files->set_v_size_flags(Tree::SIZE_EXPAND_FILL);
	staged_files->set_select_mode(Tree::SELECT_ROW);
	staged_files->connect(SNAME("item_selected"), callable_mp(this, &VersionControlEditorPlugin::_load_diff).bind(staged_files));
	staged_files->connect(SNAME("button_clicked"), callable_mp(this, &VersionControlEditorPlugin::_cell_button_pressed));
	staged_files->connect(SNAME("item_activated"), callable_mp(this, &VersionControlEditorPlugin::_item_activated).bind(staged_files));
	staged_files->create_item();
	staged_files->set_hide_root(true);
	stage_area->add_child(staged_files);

	// Editor crashes if bind is null
	unstage_all_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_move_all).bind(staged_files));
	stage_all_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_move_all).bind(unstaged_files));

	commit_dock->add_child(memnew(HSeparator));

	VBoxContainer *commit_area = memnew(VBoxContainer);
	commit_dock->add_child(commit_area);

	Label *commit_label = memnew(Label);
	commit_label->set_text(TTR("Commit Message"));
	commit_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	commit_area->add_child(commit_label);

	commit_message = memnew(TextEdit);
	commit_message->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	commit_message->set_h_grow_direction(Control::GrowDirection::GROW_DIRECTION_BEGIN);
	commit_message->set_v_grow_direction(Control::GrowDirection::GROW_DIRECTION_END);
	commit_message->set_custom_minimum_size(Size2(200, 100));
	commit_message->set_line_wrapping_mode(TextEdit::LINE_WRAPPING_BOUNDARY);
	commit_message->connect(SNAME("text_changed"), callable_mp(this, &VersionControlEditorPlugin::_update_commit_button));
	commit_message->connect(SNAME("gui_input"), callable_mp(this, &VersionControlEditorPlugin::_commit_message_gui_input));
	commit_area->add_child(commit_message);

	ED_SHORTCUT("version_control/commit", TTR("Commit"), KeyModifierMask::CMD_OR_CTRL | Key::ENTER);

	commit_button = memnew(Button);
	commit_button->set_text(TTR("Commit Changes"));
	commit_button->set_disabled(true);
	commit_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_commit));
	commit_area->add_child(commit_button);

	commit_dock->add_child(memnew(HSeparator));

	HBoxContainer *commit_list_hbc = memnew(HBoxContainer);
	commit_dock->add_child(commit_list_hbc);

	Label *commit_list_label = memnew(Label);
	commit_list_label->set_text(TTR("Commit List"));
	commit_list_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	commit_list_hbc->add_child(commit_list_label);

	commit_list_size_button = memnew(OptionButton);
	commit_list_size_button->set_tooltip_text(TTR("Commit list size"));
	commit_list_size_button->add_item("10");
	commit_list_size_button->set_item_metadata(0, 10);
	commit_list_size_button->add_item("20");
	commit_list_size_button->set_item_metadata(1, 20);
	commit_list_size_button->add_item("30");
	commit_list_size_button->set_item_metadata(2, 30);
	commit_list_size_button->connect(SNAME("item_selected"), callable_mp(this, &VersionControlEditorPlugin::_set_commit_list_size));
	commit_list_hbc->add_child(commit_list_size_button);

	commit_list = memnew(Tree);
	commit_list->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	commit_list->set_v_grow_direction(Control::GrowDirection::GROW_DIRECTION_END);
	commit_list->set_custom_minimum_size(Size2(200, 160));
	commit_list->create_item();
	commit_list->set_hide_root(true);
	commit_list->set_select_mode(Tree::SELECT_ROW);
	commit_list->set_columns(2); // Commit msg, author
	commit_list->set_column_custom_minimum_width(0, 40);
	commit_list->set_column_custom_minimum_width(1, 20);
	commit_list->connect(SNAME("item_selected"), callable_mp(this, &VersionControlEditorPlugin::_load_diff).bind(commit_list));
	commit_dock->add_child(commit_list);

	commit_dock->add_child(memnew(HSeparator));

	HBoxContainer *menu_bar = memnew(HBoxContainer);
	menu_bar->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	menu_bar->set_v_size_flags(Control::SIZE_FILL);
	commit_dock->add_child(menu_bar);

	branch_select = memnew(OptionButton);
	branch_select->set_tooltip_text(TTR("Branches"));
	branch_select->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	branch_select->connect(SNAME("item_selected"), callable_mp(this, &VersionControlEditorPlugin::_branch_item_selected));
	branch_select->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_refresh_branch_list));
	menu_bar->add_child(branch_select);

	branch_create_confirm = memnew(AcceptDialog);
	branch_create_confirm->set_title(TTR("Create New Branch"));
	branch_create_confirm->set_min_size(Size2(400, 100));
	branch_create_confirm->set_hide_on_ok(true);
	commit_dock->add_child(branch_create_confirm);

	branch_create_ok = branch_create_confirm->get_ok_button();
	branch_create_ok->set_text(TTR("Create"));
	branch_create_ok->set_disabled(true);
	branch_create_ok->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_create_branch));

	branch_remove_confirm = memnew(AcceptDialog);
	branch_remove_confirm->set_title(TTR("Remove Branch"));
	branch_remove_confirm->add_cancel_button();
	commit_dock->add_child(branch_remove_confirm);

	Button *branch_remove_ok = branch_remove_confirm->get_ok_button();
	branch_remove_ok->set_text(TTR("Remove"));
	branch_remove_ok->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_remove_branch));

	VBoxContainer *branch_create_vbc = memnew(VBoxContainer);
	branch_create_vbc->set_alignment(BoxContainer::ALIGNMENT_CENTER);
	branch_create_confirm->add_child(branch_create_vbc);

	HBoxContainer *branch_create_hbc = memnew(HBoxContainer);
	branch_create_hbc->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	branch_create_vbc->add_child(branch_create_hbc);

	Label *branch_create_name_label = memnew(Label);
	branch_create_name_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	branch_create_name_label->set_text(TTR("Branch Name"));
	branch_create_hbc->add_child(branch_create_name_label);

	branch_create_name_input = memnew(LineEdit);
	branch_create_name_input->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	branch_create_name_input->connect(SNAME("text_changed"), callable_mp(this, &VersionControlEditorPlugin::_update_branch_create_button));
	branch_create_hbc->add_child(branch_create_name_input);

	remote_select = memnew(OptionButton);
	remote_select->set_tooltip_text(TTR("Remotes"));
	remote_select->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_select->connect(SNAME("item_selected"), callable_mp(this, &VersionControlEditorPlugin::_remote_selected));
	remote_select->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_refresh_remote_list));
	menu_bar->add_child(remote_select);

	remote_create_confirm = memnew(AcceptDialog);
	remote_create_confirm->set_title(TTR("Create New Remote"));
	remote_create_confirm->set_min_size(Size2(400, 100));
	remote_create_confirm->set_hide_on_ok(true);
	commit_dock->add_child(remote_create_confirm);

	remote_create_ok = remote_create_confirm->get_ok_button();
	remote_create_ok->set_text(TTR("Create"));
	remote_create_ok->set_disabled(true);
	remote_create_ok->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_create_remote));

	remote_remove_confirm = memnew(AcceptDialog);
	remote_remove_confirm->set_title(TTR("Remove Remote"));
	remote_remove_confirm->add_cancel_button();
	commit_dock->add_child(remote_remove_confirm);

	Button *remote_remove_ok = remote_remove_confirm->get_ok_button();
	remote_remove_ok->set_text(TTR("Remove"));
	remote_remove_ok->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_remove_remote));

	VBoxContainer *remote_create_vbc = memnew(VBoxContainer);
	remote_create_vbc->set_alignment(BoxContainer::ALIGNMENT_CENTER);
	remote_create_confirm->add_child(remote_create_vbc);

	HBoxContainer *remote_create_name_hbc = memnew(HBoxContainer);
	remote_create_name_hbc->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_create_vbc->add_child(remote_create_name_hbc);

	Label *remote_create_name_label = memnew(Label);
	remote_create_name_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_create_name_label->set_text(TTR("Remote Name"));
	remote_create_name_hbc->add_child(remote_create_name_label);

	remote_create_name_input = memnew(LineEdit);
	remote_create_name_input->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_create_name_input->connect(SNAME("text_changed"), callable_mp(this, &VersionControlEditorPlugin::_update_remote_create_button));
	remote_create_name_hbc->add_child(remote_create_name_input);

	HBoxContainer *remote_create_hbc = memnew(HBoxContainer);
	remote_create_hbc->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_create_vbc->add_child(remote_create_hbc);

	Label *remote_create_url_label = memnew(Label);
	remote_create_url_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_create_url_label->set_text(TTR("Remote URL"));
	remote_create_hbc->add_child(remote_create_url_label);

	remote_create_url_input = memnew(LineEdit);
	remote_create_url_input->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	remote_create_url_input->connect(SNAME("text_changed"), callable_mp(this, &VersionControlEditorPlugin::_update_remote_create_button));
	remote_create_hbc->add_child(remote_create_url_input);

	fetch_button = memnew(Button);
	fetch_button->set_flat(true);
	fetch_button->set_tooltip_text(TTR("Fetch"));
	fetch_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("Reload"), SNAME("EditorIcons")));
	fetch_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_fetch));
	menu_bar->add_child(fetch_button);

	pull_button = memnew(Button);
	pull_button->set_flat(true);
	pull_button->set_tooltip_text(TTR("Pull"));
	pull_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("MoveDown"), SNAME("EditorIcons")));
	pull_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_pull));
	menu_bar->add_child(pull_button);

	push_button = memnew(Button);
	push_button->set_flat(true);
	push_button->set_tooltip_text(TTR("Push"));
	push_button->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("MoveUp"), SNAME("EditorIcons")));
	push_button->connect(SNAME("pressed"), callable_mp(this, &VersionControlEditorPlugin::_push));
	menu_bar->add_child(push_button);

	extra_options = memnew(MenuButton);
	extra_options->set_icon(EditorNode::get_singleton()->get_gui_base()->get_theme_icon(SNAME("GuiTabMenuHl"), SNAME("EditorIcons")));
	extra_options->get_popup()->connect(SNAME("about_to_popup"), callable_mp(this, &VersionControlEditorPlugin::_update_extra_options));
	extra_options->get_popup()->connect(SNAME("id_pressed"), callable_mp(this, &VersionControlEditorPlugin::_extra_option_selected));
	menu_bar->add_child(extra_options);

	extra_options->get_popup()->add_item(TTR("Force Push"), EXTRA_OPTION_FORCE_PUSH);
	extra_options->get_popup()->add_separator();
	extra_options->get_popup()->add_item(TTR("Create New Branch"), EXTRA_OPTION_CREATE_BRANCH);

	extra_options_remove_branch_list = memnew(PopupMenu);
	extra_options_remove_branch_list->connect(SNAME("id_pressed"), callable_mp(this, &VersionControlEditorPlugin::_popup_branch_remove_confirm));
	extra_options_remove_branch_list->set_name("Remove Branch");
	extra_options->get_popup()->add_child(extra_options_remove_branch_list);
	extra_options->get_popup()->add_submenu_item(TTR("Remove Branch"), "Remove Branch");

	extra_options->get_popup()->add_separator();
	extra_options->get_popup()->add_item(TTR("Create New Remote"), EXTRA_OPTION_CREATE_REMOTE);

	extra_options_remove_remote_list = memnew(PopupMenu);
	extra_options_remove_remote_list->connect(SNAME("id_pressed"), callable_mp(this, &VersionControlEditorPlugin::_popup_remote_remove_confirm));
	extra_options_remove_remote_list->set_name("Remove Remote");
	extra_options->get_popup()->add_child(extra_options_remove_remote_list);
	extra_options->get_popup()->add_submenu_item(TTR("Remove Remote"), "Remove Remote");
}

CommitUI::~CommitUI() {
}

} // namespace godot
