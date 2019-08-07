#include "git_api.h"

namespace godot {

bool GitAPI::is_initialized = false;
bool GitAPI::is_repo_open = false;

void GitAPI::_register_methods() {

	register_method("_process", &GitAPI::_process);

	register_method("_initialize", &GitAPI::_initialize);
	register_method("_get_vcs_name", &GitAPI::_get_vcs_name);
	register_method("_get_project_name", &GitAPI::_get_project_name);
	register_method("_commit", &GitAPI::_commit);
	register_method("_stage_all", &GitAPI::_stage_all);
	register_method("_get_commit_dock_panel_container", &GitAPI::_get_commit_dock_panel_container);
	register_method("_get_initialization_settings_panel_container", &GitAPI::_get_initialization_settings_panel_container);
	register_method("_shut_down", &GitAPI::_shut_down);
}

Object *GitAPI::_get_commit_dock_panel_container() {

	return NULL;
}

void GitAPI::_commit(const String p_msg) {

	if (!is_initialized) {

		WARN_PRINT("Set Up a VCS addon from Project Menu");
		return;
	}

}

void GitAPI::_stage_all() {

	git_index_matched_path_cb matched_cb = NULL;
	git_index *index;
	git_strarray array = { 0 };
	int options = 0, count = 0;
	StatusPayload payload;

	GIT2_CALL(git_repository_index(&index, repo), "Could not get repository index", NULL);
	ERR_FAIL_NULL(repo);

	matched_cb = &status_callback;

	payload.repo = repo;

	GIT2_CALL(git_index_add_all(index, &array, 0, matched_cb, &payload), "Could not stage in repository", NULL);
	GIT2_CALL(git_index_update_all(index, &array, matched_cb, &payload), "Could not update in repository", NULL);

	GIT2_CALL(git_index_write(index), "Could not write index object", NULL);
	git_index_free(index);
}

Object *GitAPI::_get_initialization_settings_panel_container() {

	init_settings_panel_container = memnew(PanelContainer);
	init_settings_button = memnew(Button);
	init_settings_panel_container->add_child(init_settings_button);

	return Variant(init_settings_panel_container);
}

bool GitAPI::_get_is_vcs_intialized() {

	return is_initialized;
}

String GitAPI::_get_project_name() {

	return String("project");
}

Dictionary GitAPI::_get_untracked_files_data() {

	return Dictionary();
}

String GitAPI::_get_vcs_name() {

	return "Git";
}

bool GitAPI::_initialize(const String p_project_root_path) {

	ERR_FAIL_COND_V(p_project_root_path == "", false);

	if (is_initialized == false) {

		GIT2_CALL(git_libgit2_init(), "Could not initialize libgit2", NULL);
		is_initialized = true;

	} else {

		return true;
	}

	git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_NO_REINIT;

	GIT2_CALL(git_repository_init_ext(&repo, p_project_root_path.alloc_c_string(), &opts), "Could not initialize repository", NULL);
	is_initialized = true;
	ERR_FAIL_NULL_V(repo, false);

	GIT2_CALL(git_repository_open(&repo, p_project_root_path.alloc_c_string()), "Could not open repository", false);
	is_repo_open = true;

	if (repo) {

		WARN_PRINT("Git API was initialised successfully");
	}

	return true;
}

bool GitAPI::_shut_down() {

	memdelete(init_settings_panel_container);
	memdelete(init_settings_button);

	git_repository_free(repo);
	GIT2_CALL(git_libgit2_shutdown(), "Could not shutdown Git Addon", false);

	return true;
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
