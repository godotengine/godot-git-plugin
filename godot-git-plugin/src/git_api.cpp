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

PanelContainer *GitAPI::_get_commit_dock_panel_container() {

	return NULL;
}

void GitAPI::_commit(const String p_msg) {

	if (!is_initialized) {

		WARN_PRINT("Set Up a VCS addon from Project Menu");
		return;
	}

	git_oid tree_id, parent_id, commit_id;
	git_tree *tree;
	git_commit *parent;
	char oid_hex[GIT_OID_HEXSZ + 1] = { 0 };

	GIT2_CALL(git_signature_new(&author, "test", "test@gmail.com", 123456789, 60));
	GIT2_CALL(git_signature_new(&committer, "test", "test@github.com", 987654321, 90));

	GIT2_CALL(git_oid_fromstr(&tree_id, "f60079018b664e4e79329a7ef9559c8d9e0378d1"));
	GIT2_CALL(git_tree_lookup(&tree, repo, &tree_id));
	GIT2_CALL(git_oid_fromstr(&parent_id, "5b5b025afb0b4c913b4c338a42934a3863bf3644"));
	GIT2_CALL(git_commit_lookup(&parent, repo, &parent_id));

	const char *msg = p_msg.alloc_c_string();
	git_buf *clean_msg = NULL;
	GIT2_CALL(git_message_prettify(clean_msg, msg, 0, '#'));

	GIT2_CALL(git_commit_create_v(
			&commit_id,
			repo,
			NULL,
			author,
			committer,
			NULL,
			clean_msg->ptr,
			tree,
			1,
			parent));

	git_buf_dispose(clean_msg);
}

void GitAPI::_stage_all() {
}

PanelContainer *GitAPI::_get_initialization_settings_panel_container() {

	init_settings_panel_container = memnew(PanelContainer);
	init_settings_button = memnew(Button);
	init_settings_panel_container->add_child(init_settings_button);

	return *(Variant *) (init_settings_panel_container);
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

		GIT2_CALL(git_libgit2_init() == 1, false);
		is_initialized = true;
	} else {

		return true;
	}

	git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
	opts.flags |= GIT_REPOSITORY_INIT_NO_REINIT;

	GIT2_CALL(git_repository_init_ext(&repo, p_project_root_path.alloc_c_string(), &opts) == 0, false);
	is_initialized = true;
	GIT2_CALL(git_repository_open(&repo, p_project_root_path.alloc_c_string()) == 0, false);
	is_repo_open = true;

	return true;
}

bool GitAPI::_shut_down() {

	memdelete(init_settings_panel_container);
	memdelete(init_settings_button);

	git_repository_free(repo);
	GIT2_CALL(git_libgit2_shutdown() == 0, false);

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
