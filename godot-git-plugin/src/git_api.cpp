#include "git_api.h"

namespace godot {

bool GitAPI::is_initialized = false;

void GitAPI::_register_methods() {

	register_method("_process", &GitAPI::_process);

	register_method("_initialize", &GitAPI::_initialize);
	register_method("_get_vcs_name", &GitAPI::_get_vcs_name);
	register_method("_get_project_name", &GitAPI::_get_project_name);
	register_method("_get_commit_dock_panel_container", &GitAPI::_get_commit_dock_panel_container);
	register_method("_get_initialization_settings_panel_container", &GitAPI::_get_initialization_settings_panel_container);
	register_method("_shut_down", &GitAPI::_shut_down);
}

Variant GitAPI::_get_commit_dock_panel_container() {

	return NULL;
}

Variant GitAPI::_get_initialization_settings_panel_container() {

	init_settings_panel_container = memnew(PanelContainer);

	init_settings_button = memnew(Button);
	init_settings_panel_container->add_child(init_settings_button);

	return init_settings_panel_container;
}

String GitAPI::_get_project_name() {

	return String("project");
}

String GitAPI::_get_vcs_name() {

	return "Git";
}

bool GitAPI::_initialize(const String p_project_root_path) {

	ERR_FAIL_COND_V(p_project_root_path == "", false);

	if (is_initialized == false) {

		GIT2_CALL(git_libgit2_init() == 1, false);
		is_initialized = true;
	} else {

		return true;
	}

	git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_NO_REINIT;
	GIT2_CALL(git_repository_init_ext(&repo, p_project_root_path.alloc_c_string(), &opts) == 0, false);

	GIT2_CALL(git_repository_open(&repo, p_project_root_path.alloc_c_string()) == 0, false);

	return true;
}

bool GitAPI::_shut_down() {

	memdelete(init_settings_panel_container);
	memdelete(init_settings_button);

	return true; // TODO: change with git2_shutdown() == 0
}

void GitAPI::_init() {
}

void GitAPI::_process() {
}

GitAPI::GitAPI() {
}

GitAPI::~GitAPI() {
}

} // namespace godot
