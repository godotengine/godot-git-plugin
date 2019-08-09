#include "git_api.h"

namespace godot {

bool GitAPI::is_initialized = false;

void GitAPI::_register_methods() {

	register_method("_process", &GitAPI::_process);

	register_method("_commit", &GitAPI::_commit);
	register_method("_get_commit_dock_panel_container", &GitAPI::_get_commit_dock_panel_container);
	register_method("_get_initialization_settings_panel_container", &GitAPI::_get_initialization_settings_panel_container);
	register_method("_get_is_vcs_intialized", &GitAPI::_get_is_vcs_intialized);
	register_method("_get_modified_files_data", &GitAPI::_get_modified_files_data);
	register_method("_get_project_name", &GitAPI::_get_project_name);
	register_method("_get_vcs_name", &GitAPI::_get_vcs_name);
	register_method("_initialize", &GitAPI::_initialize);
	register_method("_shut_down", &GitAPI::_shut_down);
	register_method("_stage_all", &GitAPI::_stage_all);
	register_method("_stage_file", &GitAPI::_stage_file);
}

Control *GitAPI::_get_commit_dock_panel_container() {

	return NULL;
}

void GitAPI::_commit(const String p_msg) {

	if (!is_initialized) {

		WARN_PRINT("Set Up a VCS addon from Project Menu");
		return;
	}

	git_signature *sig;
	git_index *index;
	git_oid tree_id;
	git_oid commit_id;
	git_tree *tree;

	GIT2_CALL(git_signature_default(&sig, repo), "Could not find default user signature", NULL);
	GIT2_CALL(git_repository_index(&index, repo), "Could not open repository index", NULL);
	GIT2_CALL(git_index_write_tree(&tree_id, index), "Could not write to initial tree from index", NULL);

	git_index_free(index);

	GIT2_CALL(git_tree_lookup(&tree, repo, &tree_id), "Could not lookup initial tree", NULL);
	GIT2_CALL(git_commit_create_v(&commit_id, repo, "HEAD", sig, sig, NULL, p_msg.alloc_c_string(), tree, 0), "Could not create initial commit", NULL);

	git_tree_free(tree);
	git_signature_free(sig);
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

void GitAPI::_stage_file(const String file_path) {
}

void GitAPI::create_gitignore() {

	File *file = File::_new();

	if (!file->file_exists("res://.gitignore")) {

		file->open("res://.gitignore", File::ModeFlags::WRITE);
		file->store_string(
				"# Import cache"
				"\n"
				".import/"
				"\n"
				"\n"
				"# Binaries"
				"\n"
				"bin/"
				"\n"
				"build/"
				"\n"
				"lib/"
				"\n");
		file->close();
	}
}

void GitAPI::create_initial_commit() {

	_commit("Initial commit");
}

Control *GitAPI::_get_initialization_settings_panel_container() {

	init_settings_panel_container = memnew(PanelContainer);
	init_settings_button = memnew(Button);
	init_settings_panel_container->add_child(init_settings_button);

	return Variant(init_settings_panel_container);
}

bool GitAPI::_get_is_vcs_intialized() {

	return is_initialized;
}

Dictionary GitAPI::_get_modified_files_data() {

	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	git_status_list *statuses = NULL;
	GIT2_CALL(git_status_list_new(&statuses, repo, &opts), "Could not get status information from repository", NULL);

	Dictionary diff;
	size_t count = git_status_list_entrycount(statuses);
	for (size_t i = 0; i < count; ++i) {

		const git_status_entry *entry = git_status_byindex(statuses, i);


	}

	return diff;
}

String GitAPI::_get_project_name() {

	return String("project");
}

String GitAPI::_get_vcs_name() {

	return "Git";
}

bool GitAPI::_initialize(const String p_project_root_path) {

	ERR_FAIL_COND_V(p_project_root_path == "", false);

	int init = git_libgit2_init();
	if (init > 1) {

		WARN_PRINT("Multiple libgit2 instances are running");
	}

	char *c_path = p_project_root_path.alloc_c_string();
	git_repository_init(&repo, c_path, 0); //GIT2_CALL(, "Could not initialize repository", NULL);
	if (repo) {

		WARN_PRINT("Initialized empty Git repository in res://");

		create_gitignore();
		create_initial_commit();
	}

	GIT2_CALL(git_repository_open(&repo, p_project_root_path.alloc_c_string()), "Could not open repository", false);
	if (repo) {

		WARN_PRINT("Git API was initialised successfully");
		is_initialized = true;
	}

	return is_initialized;
}

bool GitAPI::_shut_down() {

	git_repository_free(repo);
	GIT2_CALL(git_libgit2_shutdown(), "Could not shutdown Git Addon", false);

	return true;
}

void GitAPI::_init() {
}

void GitAPI::_process() {
}

GitAPI::GitAPI() {

	author = NULL;
	committer = NULL;
}

GitAPI::~GitAPI() {
}

} // namespace godot
