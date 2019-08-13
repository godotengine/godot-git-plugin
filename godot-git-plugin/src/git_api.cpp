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
	register_method("_stage_file", &GitAPI::_stage_file);
	register_method("_unstage_file", &GitAPI::_unstage_file);
}

Control *GitAPI::_get_commit_dock_panel_container() {

	return NULL;
}

void GitAPI::_commit(const String p_msg) {

	git_signature *default_sign;
	git_oid tree_id, parent_id, commit_id;
	git_tree *tree;
	git_commit *parent;
	git_index *index;

	git_signature_default(&default_sign, repo);

	git_reference_name_to_id(&parent_id, repo, "HEAD");

	/* Get the index and write it to a tree */
	git_repository_index(&index, repo);
	for (int i = 0; i < staged_files.size(); i++) {

		git_index_add_bypath(index, ((String)staged_files[i]).alloc_c_string());
	}
	git_index_write_tree(&tree_id, index);
	git_tree_lookup(&tree, repo, &tree_id);

	/* Get HEAD as a commit object to use as the parent of the commit */
	GIT2_CALL(git_reference_name_to_id(&parent_id, repo, "HEAD"), "Could not get HEAD reference for parent ID", NULL);
	git_commit_lookup(&parent, repo, &parent_id);

	/* Do the commit */
	git_commit_create_v(
			&commit_id,
			repo,
			"HEAD", /* The commit will update the position of HEAD */
			default_sign,
			default_sign,
			NULL, /* UTF-8 encoding */
			p_msg.alloc_c_string(),
			tree, /* The tree from the index */
			1, /* Only one parent */
			parent /* No need to make a list with create_v */
	);

	git_signature_free(default_sign);
	git_index_free(index);
	git_commit_free(parent);
	git_tree_free(tree);
}

void GitAPI::_stage_file(const String p_file_path) {

	if (staged_files.find(p_file_path) == -1) {

		staged_files.push_back(p_file_path);
	}
}

void GitAPI::_unstage_file(const String p_file_path) {

	staged_files.erase(p_file_path);
}

void GitAPI::create_gitignore() {

	File *file = File::_new();

	if (!file->file_exists("res://.gitignore")) {

		file->open("res://.gitignore", File::ModeFlags::WRITE);
		file->store_string(
			"# Import cache\n"
			".import/\n\n"
			"# Binaries\n"
			"bin/\n"
			"build/\n"
			"lib/\n"
		);
		file->close();
	}
}

void GitAPI::create_initial_commit() {

	git_signature *sig;
	git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;

	GIT2_CALL(git_signature_default(&sig, repo), "Unable to create a commit signature. Perhaps 'user.name' and 'user.email' are not set", NULL);
	GIT2_CALL(git_repository_index(&index, repo), "Could not create intial commit", NULL);
	GIT2_CALL(git_index_write_tree(&tree_id, index), "Could not create intial commit", NULL);

	git_index_free(index);

	GIT2_CALL(git_tree_lookup(&tree, repo, &tree_id), "Could not create intial commit", NULL);
	GIT2_CALL(
		git_commit_create_v(
					&commit_id,
					repo,
					"HEAD",
					sig,
					sig,
					NULL,
					"Initial commit",
					tree,
					0
		),
		"Could not create the initial commit",
		NULL);

	git_tree_free(tree);
	git_signature_free(sig);
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
	opts.flags += GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
	opts.flags += GIT_STATUS_OPT_INCLUDE_UNTRACKED;
	opts.flags += GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;
	opts.flags += GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX;
	git_status_list *statuses = NULL;

	GIT2_CALL(git_status_list_new(&statuses, repo, &opts), "Could not get status information from repository", NULL);

	Dictionary diff; // Schema is <file_path, status>
	size_t count = git_status_list_entrycount(statuses);
	for (size_t i = 0; i < count; ++i) {

		const git_status_entry *entry = git_status_byindex(statuses, i);
		String path;
		if (entry->index_to_workdir) {

			path = entry->index_to_workdir->new_file.path;
		} else {

			path = entry->head_to_index->new_file.path;
		}
		switch (entry->status) {

			case GIT_STATUS_INDEX_NEW:
			case GIT_STATUS_WT_NEW: {

				diff[path] = 0;
			} break;
			case GIT_STATUS_INDEX_MODIFIED:
			case GIT_STATUS_WT_MODIFIED: {

				diff[path] = 1;
			} break;
			case GIT_STATUS_INDEX_RENAMED:
			case GIT_STATUS_WT_RENAMED: {

				diff[path] = 2;
			} break;
			case GIT_STATUS_INDEX_DELETED:
			case GIT_STATUS_WT_DELETED: {

				diff[path] = 3;
			} break;
			case GIT_STATUS_INDEX_TYPECHANGE:
			case GIT_STATUS_WT_TYPECHANGE: {

				diff[path] = 4;
			} break;
		}
	}

	git_status_list_free(statuses);

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

	if (is_initialized) {

		return true;
	}

	GIT2_CALL(git_repository_init(&repo, p_project_root_path.alloc_c_string(), 0), "Could not initialize repository", NULL);
	if (git_repository_head_unborn(repo) == 1) {

		create_gitignore();
		create_initial_commit();
	}

	GIT2_CALL(git_repository_open(&repo, p_project_root_path.alloc_c_string()), "Could not open repository", NULL);
	is_initialized = true;

	return is_initialized;
}

bool GitAPI::_shut_down() {

	git_repository_free(repo);
	GIT2_CALL(git_libgit2_shutdown(), "Could not shutdown Git Addon", NULL);

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
