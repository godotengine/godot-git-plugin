#include "git_api.h"

#define GIT2_CALL(error, msg)                                         \
	if (check_errors(error, __FUNCTION__, __FILE__, __LINE__, msg)) { \
		return;                                                       \
	}

#define GIT2_CALL_R(error, msg, return_value)                         \
	if (check_errors(error, __FUNCTION__, __FILE__, __LINE__, msg)) { \
		return return_value;                                          \
	}

#define GIT2_CALL_IGNORE(error, msg, ignores)                                  \
	if (check_errors(error, __FUNCTION__, __FILE__, __LINE__, msg, ignores)) { \
		return;                                                                \
	}

#define GIT2_CALL_R_IGNORE(error, msg, return_value, ignores)                  \
	if (check_errors(error, __FUNCTION__, __FILE__, __LINE__, msg, ignores)) { \
		return return_value;                                                   \
	}

#define COMMA ,

namespace godot {

void GitAPI::_register_methods() {
	register_method("_commit", &GitAPI::_commit);
	register_method("_get_modified_files_data", &GitAPI::_get_modified_files_data);
	register_method("_get_remotes", &GitAPI::_get_remotes);
	register_method("_get_diff", &GitAPI::_get_diff);
	register_method("_get_vcs_name", &GitAPI::_get_vcs_name);
	register_method("_initialize", &GitAPI::_initialize);
	register_method("_shut_down", &GitAPI::_shut_down);
	register_method("_stage_file", &GitAPI::_stage_file);
	register_method("_discard_file", &GitAPI::_discard_file);
	register_method("_unstage_file", &GitAPI::_unstage_file);
	register_method("_get_previous_commits", &GitAPI::_get_previous_commits);
	register_method("_get_branch_list", &GitAPI::_get_branch_list);
	register_method("_create_branch", &GitAPI::_create_branch);
	register_method("_create_remote", &GitAPI::_create_remote);
	register_method("_remove_branch", &GitAPI::_remove_branch);
	register_method("_remove_remote", &GitAPI::_remove_remote);
	register_method("_get_current_branch_name", &GitAPI::_get_current_branch_name);
	register_method("_checkout_branch", &GitAPI::_checkout_branch);
	register_method("_fetch", &GitAPI::_fetch);
	register_method("_pull", &GitAPI::_pull);
	register_method("_push", &GitAPI::_push);
	register_method("_get_line_diff", &GitAPI::_get_line_diff);
	register_method("_set_credentials", &GitAPI::_set_credentials);
}

bool GitAPI::check_errors(int error, String function, String file, int line, String message, const std::vector<git_error_code> &ignores) {
	const git_error *lg2err;

	if (error == 0) {
		return false;
	}

	for (auto &ig : ignores) {
		if (error == ig) {
			return false;
		}
	}

	message += ".";
	if ((lg2err = git_error_last()) != nullptr && lg2err->message != nullptr) {
		message += " Error " + String::num_int64(error) + ": ";
		message += String(lg2err->message);
	}

	Godot::print_error("GitAPI: " + message, function, file, line);
	popup_error(message);
	return true;
}

void GitAPI::_set_credentials(const String username, const String password, const String ssh_public_key_path, const String ssh_private_key_path, const String ssh_passphrase) {
	creds.username = username;
	creds.password = password;
	creds.ssh_public_key_path = ssh_public_key_path;
	creds.ssh_private_key_path = ssh_private_key_path;
	creds.ssh_passphrase = ssh_passphrase;
}

void GitAPI::_discard_file(const String file_path) {
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	CString c_path(file_path);
	char *paths[] = { c_path.data };
	opts.paths = { paths, 1 };
	opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	GIT2_CALL(git_checkout_index(repo.get(), nullptr, &opts), "Could not checkout index");
}

void GitAPI::_commit(const String msg) {
	git_index_ptr repo_index;
	GIT2_CALL(git_repository_index(Capture(repo_index), repo.get()), "Could not get repository index");

	git_oid tree_id;
	GIT2_CALL(git_index_write_tree(&tree_id, repo_index.get()), "Could not write index to tree");
	GIT2_CALL(git_index_write(repo_index.get()), "Could not write index to disk");

	git_tree_ptr tree;
	GIT2_CALL(git_tree_lookup(Capture(tree), repo.get(), &tree_id), "Could not lookup tree from ID");

	git_signature_ptr default_sign;
	GIT2_CALL(git_signature_default(Capture(default_sign), repo.get()), "Could not get default signature");

	git_oid parent_commit_id = {};
	GIT2_CALL_IGNORE(git_reference_name_to_id(&parent_commit_id, repo.get(), "HEAD"), "Could not get repository HEAD", { GIT_ENOTFOUND });

	git_commit_ptr parent_commit;
	if (!git_oid_is_zero(&parent_commit_id)) {
		GIT2_CALL(git_commit_lookup(Capture(parent_commit), repo.get(), &parent_commit_id), "Could not lookup parent commit data");
	}

	git_oid new_commit_id;
	if (!has_merge) {
		GIT2_CALL(
				git_commit_create_v(
						&new_commit_id,
						repo.get(),
						"HEAD",
						default_sign.get(),
						default_sign.get(),
						"UTF-8",
						CString(msg).data,
						tree.get(),
						parent_commit.get() ? 1 : 0,
						parent_commit.get()),
				"Could not create commit");
	} else {
		git_commit_ptr fetchhead_commit;
		GIT2_CALL(git_commit_lookup(Capture(fetchhead_commit), repo.get(), &pull_merge_oid), "Could not lookup commit pointed to by HEAD");

		GIT2_CALL(
				git_commit_create_v(
						&new_commit_id,
						repo.get(),
						"HEAD",
						default_sign.get(),
						default_sign.get(),
						"UTF-8",
						CString(msg).data,
						tree.get(),
						2,
						parent_commit.get(),
						fetchhead_commit.get()),
				"Could not create merge commit");
		has_merge = false;
		GIT2_CALL(git_repository_state_cleanup(repo.get()), "Could not clean repository state");
	}
}

void GitAPI::_stage_file(const String file_path) {
	CString c_path(file_path);
	char *paths[] = { c_path.data };
	git_strarray array = { paths, 1 };

	git_index_ptr index;
	GIT2_CALL(git_repository_index(Capture(index), repo.get()), "Could not get repository index");
	GIT2_CALL(git_index_add_all(index.get(), &array, GIT_INDEX_ADD_DEFAULT | GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, nullptr, nullptr), "Could not add " + file_path + " to index");
	GIT2_CALL(git_index_write(index.get()), "Could not write changes to disk");
}

void GitAPI::_unstage_file(const String file_path) {
	CString c_path(file_path);
	char *paths[] = { c_path.data };
	git_strarray array = { paths, 1 };

	git_reference_ptr head;
	GIT2_CALL_IGNORE(git_repository_head(Capture(head), repo.get()), "Could not find repository HEAD", { GIT_ENOTFOUND COMMA GIT_EUNBORNBRANCH });

	if (head) {
		git_object_ptr head_commit;
		GIT2_CALL(git_reference_peel(Capture(head_commit), head.get(), GIT_OBJ_COMMIT), "Could not peel HEAD reference");
		GIT2_CALL(git_reset_default(repo.get(), head_commit.get(), &array), "Could not reset " + file_path + " to state at HEAD");
	} else {
		// If there is no HEAD commit, we should just remove the file from the index.

		CString c_path(file_path);

		git_index_ptr index;
		GIT2_CALL(git_repository_index(Capture(index), repo.get()), "Could not get repository index");
		GIT2_CALL(git_index_remove_bypath(index.get(), c_path.data), "Could not add " + file_path + " to index");
		GIT2_CALL(git_index_write(index.get()), "Could not write changes to disk");
	}
}

void GitAPI::create_gitignore_and_gitattributes() {
	File *file = File::_new();

	if (!file->file_exists("res://.gitignore")) {
		file->open("res://.gitignore", File::ModeFlags::WRITE);
		file->store_string(
				"# Godot-specific ignores\n"
				".import/\n"
				"export.cfg\n"
				"export_presets.cfg\n\n"

				"# Imported translations (automatically generated from CSV files)\n"
				"*.translation\n\n"

				"# Mono-specific ignores\n"
				".mono/\n"
				"data_*/\n");
		file->close();
	}

	if (!file->file_exists("res://.gitattributes")) {
		file->open("res://.gitattributes", File::ModeFlags::WRITE);
		file->store_string(
				"# Set the default behavior, in case people don't have core.autocrlf set.\n"
				"* text=auto\n\n"

				"# Explicitly declare text files you want to always be normalized and converted\n"
				"# to native line endings on checkout.\n"
				"*.cpp text\n"
				"*.c text\n"
				"*.h text\n"
				"*.gd text\n"
				"*.cs text\n\n"

				"# Declare files that will always have CRLF line endings on checkout.\n"
				"*.sln text eol=crlf\n\n"

				"# Denote all files that are truly binary and should not be modified.\n"
				"*.png binary\n"
				"*.jpg binary\n");
		file->close();
	}

	file->free();
}

Array GitAPI::_get_modified_files_data() {
	Array stats_files;

	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	opts.flags = GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
	opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

	git_status_list_ptr statuses;
	GIT2_CALL_R(git_status_list_new(Capture(statuses), repo.get(), &opts), "Could not get status information from repository", Array());

	size_t count = git_status_list_entrycount(statuses.get());
	for (size_t i = 0; i < count; ++i) {
		const git_status_entry *entry = git_status_byindex(statuses.get(), i);
		String path;
		if (entry->index_to_workdir) {
			path = entry->index_to_workdir->new_file.path;
		} else {
			path = entry->head_to_index->new_file.path;
		}

		static Dictionary map_changes = Dictionary::make(
				GIT_STATUS_WT_NEW, CHANGE_TYPE_NEW,
				GIT_STATUS_INDEX_NEW, CHANGE_TYPE_NEW,
				GIT_STATUS_WT_MODIFIED, CHANGE_TYPE_MODIFIED,
				GIT_STATUS_INDEX_MODIFIED, CHANGE_TYPE_MODIFIED,
				GIT_STATUS_WT_RENAMED, CHANGE_TYPE_RENAMED,
				GIT_STATUS_INDEX_RENAMED, CHANGE_TYPE_RENAMED,
				GIT_STATUS_WT_DELETED, CHANGE_TYPE_DELETED,
				GIT_STATUS_INDEX_DELETED, CHANGE_TYPE_DELETED,
				GIT_STATUS_WT_TYPECHANGE, CHANGE_TYPE_TYPECHANGE,
				GIT_STATUS_INDEX_TYPECHANGE, CHANGE_TYPE_TYPECHANGE,
				GIT_STATUS_CONFLICTED, CHANGE_TYPE_UNMERGED);

		const static int git_status_wt = GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_CONFLICTED;
		const static int git_status_index = GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE;

		if (entry->status & git_status_wt) {
			stats_files.push_back(create_status_file(path, map_changes[entry->status & git_status_wt], TREE_AREA_UNSTAGED));
		}

		if (entry->status & git_status_index) {
			if (entry->status == GIT_STATUS_INDEX_RENAMED) {
				String old_path = entry->head_to_index->old_file.path;
				stats_files.push_back(create_status_file(old_path, map_changes[GIT_STATUS_INDEX_DELETED], TREE_AREA_STAGED));
				stats_files.push_back(create_status_file(path, map_changes[GIT_STATUS_INDEX_NEW], TREE_AREA_STAGED));
			} else {
				stats_files.push_back(create_status_file(path, map_changes[entry->status & git_status_index], TREE_AREA_STAGED));
			}
		}
	}

	return stats_files;
}

Array GitAPI::_get_branch_list() {
	git_branch_iterator_ptr it;
	GIT2_CALL_R(git_branch_iterator_new(Capture(it), repo.get(), GIT_BRANCH_LOCAL), "Could not create branch iterator", Array());

	Array branch_names;

	git_reference_ptr ref;
	git_branch_t type;
	while (git_branch_next(Capture(ref), &type, it.get()) != GIT_ITEROVER) {
		const char *name = nullptr;

		GIT2_CALL_R(git_branch_name(&name, ref.get()), "Could not get branch name", Array());

		if (git_branch_is_head(ref.get())) {
			// Always send the current branch as the first branch in list
			branch_names.push_front(name);
		} else {
			branch_names.push_back(String(name));
		}
	}

	return branch_names;
}

void GitAPI::_create_branch(const String branch_name) {
	git_oid head_commit_id;
	GIT2_CALL(git_reference_name_to_id(&head_commit_id, repo.get(), "HEAD"), "Could not get HEAD commit ID");

	git_commit_ptr head_commit;
	GIT2_CALL(git_commit_lookup(Capture(head_commit), repo.get(), &head_commit_id), "Could not lookup HEAD commit");

	git_reference_ptr branch_ref;
	GIT2_CALL(git_branch_create(Capture(branch_ref), repo.get(), CString(branch_name).data, head_commit.get(), 0), "Could not create branch from HEAD");
}

void GitAPI::_create_remote(const String remote_name, const String remote_url) {
	git_remote_ptr remote;
	GIT2_CALL(git_remote_create(Capture(remote), repo.get(), CString(remote_name).data, CString(remote_url).data), "Could not create remote");
}

void GitAPI::_remove_branch(const String branch_name) {
	git_reference_ptr branch;
	GIT2_CALL(git_branch_lookup(Capture(branch), repo.get(), CString(branch_name).data, GIT_BRANCH_LOCAL), "Could not find branch " + branch_name);
	GIT2_CALL(git_branch_delete(branch.get()), "Could not delete branch reference of " + branch_name);
}

void GitAPI::_remove_remote(const String remote_name) {
	GIT2_CALL(git_remote_delete(repo.get(), CString(remote_name).data), "Could not delete remote " + remote_name);
}

Array GitAPI::_get_line_diff(String file_path, String text) {
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;

	opts.context_lines = 0;
	opts.flags = GIT_DIFF_DISABLE_PATHSPEC_MATCH | GIT_DIFF_INCLUDE_UNTRACKED;

	git_index_ptr index;
	GIT2_CALL_R(git_repository_index(Capture(index), repo.get()), "Failed to get repository index", Array());
	GIT2_CALL_R(git_index_read(index.get(), 0), "Failed to read index", Array());

	const git_index_entry *entry = git_index_get_bypath(index.get(), CString(file_path).data, GIT_INDEX_STAGE_NORMAL);
	if (!entry) {
		return Array();
	}

	git_reference_ptr head;
	GIT2_CALL_R(git_repository_head(Capture(head), repo.get()), "Failed to load repository head", Array());

	git_blob_ptr blob;
	GIT2_CALL_R(git_blob_lookup(Capture(blob), repo.get(), &entry->id), "Failed to load head blob", Array());

	Array diff_contents;

	DiffHelper diff_helper = { &diff_contents, this };
	GIT2_CALL_R(git_diff_blob_to_buffer(blob.get(), nullptr, CString(text).data, text.length(), nullptr, &opts, nullptr, nullptr, diff_hunk_cb, nullptr, &diff_helper), "Failed to make diff", Array());

	return diff_contents;
}

String GitAPI::_get_current_branch_name() {
	git_reference_ptr head;
	GIT2_CALL_R_IGNORE(git_repository_head(Capture(head), repo.get()), "Could not find repository HEAD", "", { GIT_ENOTFOUND COMMA GIT_EUNBORNBRANCH });

	if (!head) {
		// We are likely at a state where the initial commit hasn't been made yet.
		return "";
	}

	git_reference_ptr branch;
	GIT2_CALL_R(git_reference_resolve(Capture(branch), head.get()), "Could not resolve HEAD reference", "");

	const char *name = "";
	GIT2_CALL_R(git_branch_name(&name, branch.get()), "Could not get branch name from current branch reference", "");

	return name;
}

Array GitAPI::_get_remotes() {
	git_strarray remote_array;
	GIT2_CALL_R(git_remote_list(&remote_array, repo.get()), "Could not get list of remotes", Array());

	Array remotes;
	for (int i = 0; i < remote_array.count; i++) {
		remotes.push_back(remote_array.strings[i]);
	}

	return remotes;
}

Array GitAPI::_get_previous_commits(const int64_t max_commits) {
	git_revwalk_ptr walker;
	GIT2_CALL_R(git_revwalk_new(Capture(walker), repo.get()), "Could not create new revwalk", Array());
	GIT2_CALL_R(git_revwalk_sorting(walker.get(), GIT_SORT_TIME), "Could not sort revwalk by time", Array());

	GIT2_CALL_R_IGNORE(git_revwalk_push_head(walker.get()), "Could not push HEAD to revwalk", Array(), { GIT_ENOTFOUND COMMA GIT_ERROR });

	git_oid oid;
	Array commits;
	char commit_id[GIT_OID_HEXSZ + 1];
	for (int i = 0; !git_revwalk_next(&oid, walker.get()) && i <= max_commits; i++) {
		git_commit_ptr commit;
		GIT2_CALL_R(git_commit_lookup(Capture(commit), repo.get(), &oid), "Failed to lookup the commit", commits);

		git_oid_tostr(commit_id, GIT_OID_HEXSZ + 1, git_commit_id(commit.get()));
		String msg = git_commit_message(commit.get());

		const git_signature *sig = git_commit_author(commit.get());
		String author = String() + sig->name + " <" + sig->email + ">";

		commits.push_back(create_commit(msg, author, commit_id, sig->when.time, sig->when.offset));
	}

	return commits;
}

void GitAPI::_fetch(String remote) {
	Godot::print("GitAPI: Performing fetch from " + remote);

	git_remote_ptr remote_object;
	GIT2_CALL(git_remote_lookup(Capture(remote_object), repo.get(), CString(remote).data), "Could not lookup remote \"" + remote + "\"");

	git_remote_callbacks remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = &credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = &transfer_progress_cb;
	remote_cbs.payload = &creds;
	remote_cbs.push_transfer_progress = &push_transfer_progress_cb;
	remote_cbs.push_update_reference = &push_update_reference_cb;

	GIT2_CALL(git_remote_connect(remote_object.get(), GIT_DIRECTION_FETCH, &remote_cbs, nullptr, nullptr), "Could not connect to remote \"" + remote + "\". Are your credentials correct? Try using a PAT token (in case you are using Github) as your password");

	git_fetch_options opts = GIT_FETCH_OPTIONS_INIT;
	opts.callbacks = remote_cbs;
	GIT2_CALL(git_remote_fetch(remote_object.get(), nullptr, &opts, "fetch"), "Could not fetch data from remote");

	Godot::print("GitAPI: Fetch ended");
}

void GitAPI::_pull(String remote) {
	Godot::print("GitAPI: Performing pull from " + remote);

	git_remote_ptr remote_object;
	GIT2_CALL(git_remote_lookup(Capture(remote_object), repo.get(), CString(remote).data), "Could not lookup remote \"" + remote + "\"");

	git_remote_callbacks remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = &credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = &transfer_progress_cb;
	remote_cbs.payload = &creds;
	remote_cbs.push_transfer_progress = &push_transfer_progress_cb;
	remote_cbs.push_update_reference = &push_update_reference_cb;

	GIT2_CALL(git_remote_connect(remote_object.get(), GIT_DIRECTION_FETCH, &remote_cbs, nullptr, nullptr), "Could not connect to remote \"" + remote + "\". Are your credentials correct? Try using a PAT token (in case you are using Github) as your password");

	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	fetch_opts.callbacks = remote_cbs;

	String branch_name = _get_current_branch_name();

	CString ref_spec_str("refs/heads/" + branch_name);

	char *ref[] = { ref_spec_str.data };
	git_strarray refspec = { ref, 1 };

	GIT2_CALL(git_remote_fetch(remote_object.get(), &refspec, &fetch_opts, "pull"), "Could not fetch data from remote");

	pull_merge_oid = {};
	GIT2_CALL(git_repository_fetchhead_foreach(repo.get(), fetchhead_foreach_cb, &pull_merge_oid), "Could not read \"FETCH_HEAD\" file");

	if (git_oid_is_zero(&pull_merge_oid)) {
		popup_error("GitAPI: Could not find remote branch HEAD for " + branch_name + ". Try pushing the branch first.");
		return;
	}

	git_annotated_commit_ptr fetchhead_annotated_commit;
	GIT2_CALL(git_annotated_commit_lookup(Capture(fetchhead_annotated_commit), repo.get(), &pull_merge_oid), "Could not get merge commit");

	const git_annotated_commit *merge_heads[] = { fetchhead_annotated_commit.get() };

	git_merge_analysis_t merge_analysis;
	git_merge_preference_t preference = GIT_MERGE_PREFERENCE_NONE;
	GIT2_CALL(git_merge_analysis(&merge_analysis, &preference, repo.get(), merge_heads, 1), "Merge analysis failed");

	if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
		git_checkout_options ff_checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
		int err = 0;

		git_reference_ptr target_ref;
		GIT2_CALL(git_repository_head(Capture(target_ref), repo.get()), "Failed to get HEAD reference");

		git_object_ptr target;
		GIT2_CALL(git_object_lookup(Capture(target), repo.get(), &pull_merge_oid, GIT_OBJECT_COMMIT), "Failed to lookup OID " + String(git_oid_tostr_s(&pull_merge_oid)));

		ff_checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
		GIT2_CALL(git_checkout_tree(repo.get(), target.get(), &ff_checkout_options), "Failed to checkout HEAD reference");

		git_reference_ptr new_target_ref;
		GIT2_CALL(git_reference_set_target(Capture(new_target_ref), target_ref.get(), &pull_merge_oid, nullptr), "Failed to move HEAD reference");

		Godot::print("GitAPI: Fast Forwarded");
		GIT2_CALL(git_repository_state_cleanup(repo.get()), "Could not clean repository state");

	} else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
		git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
		git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

		merge_opts.file_favor = GIT_MERGE_FILE_FAVOR_NORMAL;
		merge_opts.file_flags = (GIT_MERGE_FILE_STYLE_DIFF3 | GIT_MERGE_FILE_DIFF_MINIMAL);
		checkout_opts.checkout_strategy = (GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS | GIT_CHECKOUT_CONFLICT_STYLE_MERGE);
		GIT2_CALL(git_merge(repo.get(), merge_heads, 1, &merge_opts, &checkout_opts), "Merge Failed");

		git_index_ptr index;
		GIT2_CALL(git_repository_index(Capture(index), repo.get()), "Could not get repository index");

		if (git_index_has_conflicts(index.get())) {
			popup_error("GitAPI: Index has conflicts, Solve conflicts and make a merge commit.");
		} else {
			popup_error("GitAPI: Change are staged, make a merge commit.");
		}

		has_merge = true;

	} else if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
		Godot::print("GitAPI: Already up to date");

		GIT2_CALL(git_repository_state_cleanup(repo.get()), "Could not clean repository state");

	} else {
		Godot::print("GitAPI: Can not merge");
	}

	Godot::print("GitAPI: Pull ended");
}

void GitAPI::_push(const String remote, const bool force) {
	Godot::print("GitAPI: Performing push to " + remote);

	git_remote_ptr remote_object;
	GIT2_CALL(git_remote_lookup(Capture(remote_object), repo.get(), CString(remote).data), "Could not lookup remote \"" + remote + "\"");

	git_remote_callbacks remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = &credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = &transfer_progress_cb;
	remote_cbs.payload = &creds;
	remote_cbs.push_transfer_progress = &push_transfer_progress_cb;
	remote_cbs.push_update_reference = &push_update_reference_cb;

	GIT2_CALL(git_remote_connect(remote_object.get(), GIT_DIRECTION_PUSH, &remote_cbs, nullptr, nullptr), "Could not connect to remote \"" + remote + "\". Are your credentials correct? Try using a PAT token (in case you are using Github) as your password");

	String branch_name = _get_current_branch_name();

	CString pushspec(String() + (force ? "+" : "") + "refs/heads/" + branch_name);
	const git_strarray refspec = { &pushspec.data, 1 };

	git_push_options push_options = GIT_PUSH_OPTIONS_INIT;
	push_options.callbacks = remote_cbs;

	GIT2_CALL(git_remote_push(remote_object.get(), &refspec, &push_options), "Failed to push");

	Godot::print("GitAPI: Push ended");
}

bool GitAPI::_checkout_branch(String branch_name) {
	git_reference_ptr branch;
	GIT2_CALL_R(git_branch_lookup(Capture(branch), repo.get(), CString(branch_name).data, GIT_BRANCH_LOCAL), "Could not find branch", false);
	const char *branch_ref_name = git_reference_name(branch.get());

	git_object_ptr treeish;
	GIT2_CALL_R(git_revparse_single(Capture(treeish), repo.get(), CString(branch_name).data), "Could not find branch head", false);

	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	GIT2_CALL_R(git_checkout_tree(repo.get(), treeish.get(), &opts), "Could not checkout branch tree", false);
	GIT2_CALL_R(git_repository_set_head(repo.get(), branch_ref_name), "Could not set head", false);

	return true;
}

Array GitAPI::_get_diff(const String identifier, const int64_t area) {
	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	Array diff_contents;

	opts.context_lines = 2;
	opts.interhunk_lines = 0;
	opts.flags = GIT_DIFF_RECURSE_UNTRACKED_DIRS | GIT_DIFF_DISABLE_PATHSPEC_MATCH | GIT_DIFF_INCLUDE_UNTRACKED | GIT_DIFF_SHOW_UNTRACKED_CONTENT | GIT_DIFF_INCLUDE_TYPECHANGE;

	CString pathspec(identifier);
	opts.pathspec.strings = &pathspec.data;
	opts.pathspec.count = 1;

	git_diff_ptr diff;
	switch ((TreeArea)area) {
		case TREE_AREA_UNSTAGED: {
			GIT2_CALL_R(git_diff_index_to_workdir(Capture(diff), repo.get(), nullptr, &opts), "Could not create diff for index from working directory", diff_contents);
		} break;
		case TREE_AREA_STAGED: {
			git_object_ptr obj;

			// Ignore the case when HEAD is not found. We need to compare with a null tree in the case where the HEAD reference object is empty.
			GIT2_CALL_R_IGNORE(git_revparse_single(Capture(obj), repo.get(), "HEAD^{tree}"), "Could not get HEAD^{tree} object", diff_contents, { GIT_ENOTFOUND });

			git_tree_ptr tree;
			if (obj) {
				GIT2_CALL_R_IGNORE(git_tree_lookup(Capture(tree), repo.get(), git_object_id(obj.get())), "Could not lookup HEAD^{tree}", diff_contents, { GIT_ENOTFOUND });
			}

			GIT2_CALL_R(git_diff_tree_to_index(Capture(diff), repo.get(), tree.get(), nullptr, &opts), "Could not create diff for tree from index directory", diff_contents);
		} break;
		case TREE_AREA_COMMIT: {
			opts.pathspec = {};

			git_object_ptr obj;
			GIT2_CALL_R(git_revparse_single(Capture(obj), repo.get(), pathspec.data), "Could not get object at " + identifier, diff_contents);

			git_commit_ptr commit;
			GIT2_CALL_R(git_commit_lookup(Capture(commit), repo.get(), git_object_id(obj.get())), "Could not get commit " + identifier, diff_contents);

			git_commit_ptr parent;

			// We ignore the case when the parent is not found to handle the case when this commit is the root commit. We only need to diff against a null tree in that case.
			GIT2_CALL_R_IGNORE(git_commit_parent(Capture(parent), commit.get(), 0), "Could not get parent commit of " + identifier, diff_contents, { GIT_ENOTFOUND });

			git_tree_ptr commit_tree;
			GIT2_CALL_R(git_commit_tree(Capture(commit_tree), commit.get()), "Could not get commit tree of " + identifier, diff_contents);

			git_tree_ptr parent_tree;
			if (parent) {
				GIT2_CALL_R(git_commit_tree(Capture(parent_tree), parent.get()), "Could not get commit tree of " + identifier, diff_contents);
			}

			GIT2_CALL_R(git_diff_tree_to_tree(Capture(diff), repo.get(), parent_tree.get(), commit_tree.get(), &opts), "Could not generate diff for commit " + identifier, diff_contents);
		} break;
	}

	diff_contents = _parse_diff(diff.get());

	return diff_contents;
}

Array GitAPI::_parse_diff(git_diff *diff) {
	Array diff_contents;
	for (int i = 0; i < git_diff_num_deltas(diff); i++) {
		const git_diff_delta *delta = git_diff_get_delta(diff, i);

		git_patch_ptr patch;
		GIT2_CALL_R(git_patch_from_diff(Capture(patch), diff, i), "Could not create patch from diff", Array());

		Dictionary diff_file = create_diff_file(delta->new_file.path, delta->old_file.path);

		Array diff_hunks;
		for (int j = 0; j < git_patch_num_hunks(patch.get()); j++) {
			const git_diff_hunk *git_hunk;
			size_t line_count;
			GIT2_CALL_R(git_patch_get_hunk(&git_hunk, &line_count, patch.get(), j), "Could not get hunk from patch", Array());

			Dictionary diff_hunk = create_diff_hunk(git_hunk->old_start, git_hunk->new_start, git_hunk->old_lines, git_hunk->new_lines);

			Array diff_lines;
			for (int k = 0; k < line_count; k++) {
				const git_diff_line *git_diff_line;
				GIT2_CALL_R(git_patch_get_line_in_hunk(&git_diff_line, patch.get(), j, k), "Could not get line from hunk in patch", Array());

				char *content = new char[git_diff_line->content_len + 1];
				memcpy(content, git_diff_line->content, git_diff_line->content_len);
				content[git_diff_line->content_len] = '\0';

				diff_lines.push_back(create_diff_line(git_diff_line->new_lineno, git_diff_line->old_lineno, String(content), String(git_diff_line->origin)));

				delete[] content;
			}

			diff_hunk = add_line_diffs_into_diff_hunk(diff_hunk, diff_lines);
			diff_hunks.push_back(diff_hunk);
		}
		diff_file = add_diff_hunks_into_diff_file(diff_file, diff_hunks);
		diff_contents.push_back(diff_file);
	}
	return diff_contents;
}

String GitAPI::_get_vcs_name() {
	return "Git";
}

bool GitAPI::_initialize(String project_path) {
	ERR_FAIL_COND_V(project_path == "", false);

	int init = git_libgit2_init();
	if (init > 1) {
		WARN_PRINT("Multiple libgit2 instances are running");
	}

	GIT2_CALL_R(git_repository_init(Capture(repo), CString(project_path).data, 0), "Could not initialize repository", false);

	git_reference_ptr head;
	GIT2_CALL_R_IGNORE(git_repository_head(Capture(head), repo.get()), "Could not get repository HEAD", false, { GIT_EUNBORNBRANCH COMMA GIT_ENOTFOUND });

	if (!head) {
		create_gitignore_and_gitattributes();
	}

	return true;
}

bool GitAPI::_shut_down() {
	repo.reset(); // Destroy repo object before libgit2 shuts down
	GIT2_CALL_R(git_libgit2_shutdown(), "Could not shutdown Git Plugin", false);
	return true;
}

void GitAPI::_init() {
}

} // namespace godot
