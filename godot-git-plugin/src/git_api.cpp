#include "git_api.h"

#define GIT2_CALL_R(error_msg, return_value, function, ...)                                 \
	if (check_errors(function(__VA_ARGS__), error_msg, __FUNCTION__, __FILE__, __LINE__))   \
	{                                                                                       \
		return return_value;                                                                \
	}

#define GIT2_CALL(error_msg, function, ...) GIT2_CALL_R(error_msg, , function, __VA_ARGS__)

#define GIT2_PTR_R(error_msg, return_value, function, smart_ptr, ...)  \
do {                                                                   \
	decltype(smart_ptr.get()) ptr = nullptr;                           \
	GIT2_CALL_R(error_msg, return_value, function, &ptr, __VA_ARGS__); \
    smart_ptr.reset(ptr);                                              \
} while(0)

#define GIT2_PTR(error_msg, function, smart_ptr, ...) GIT2_PTR_R(error_msg, , function, smart_ptr, __VA_ARGS__)

namespace godot {

void GitAPI::_register_methods() {
	register_method("_process", &GitAPI::_process);

	register_method("_commit", &GitAPI::_commit);
	register_method("_is_vcs_initialized", &GitAPI::_is_vcs_initialized);
	register_method("_get_modified_files_data", &GitAPI::_get_modified_files_data);
	register_method("_get_remotes", &GitAPI::_get_remotes);
	register_method("_get_file_diff", &GitAPI::_get_file_diff);
	register_method("_get_project_name", &GitAPI::_get_project_name);
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
	register_method("_get_current_branch_name", &GitAPI::_get_current_branch_name);
	register_method("_checkout_branch", &GitAPI::_checkout_branch);
	register_method("_fetch", &GitAPI::_fetch);
	register_method("_pull", &GitAPI::_pull);
	register_method("_push", &GitAPI::_push);
	register_method("_get_line_diff", &GitAPI::_get_line_diff);
}

bool GitAPI::check_errors(int error, String message, String function, String file, int line) {

	const git_error *lg2err;

	if (!error) {

		return false;
	}

	message += ".";
	if ((lg2err = git_error_last()) != NULL && lg2err->message != NULL) {
		message += " Error " + String::num_int64(error) + ": ";
		message += String(lg2err->message);
	}
	Godot::print_error("GitAPI: " + message, function, file, line);
	popup_error(message);
	return true;
}

void GitAPI::_discard_file(const String file_path) {

	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	CString c_path(file_path);
	char *paths[] = { c_path.data };
	opts.paths = { paths, 1 };
	opts.checkout_strategy = GIT_CHECKOUT_FORCE;
	GIT2_CALL("Cannot checkout a file", 
		git_checkout_index, repo.get(), NULL, &opts);
}

void GitAPI::_commit(const String msg) {
	git_index_ptr repo_index;
	GIT2_PTR("Could not get repository index", 
		git_repository_index, repo_index, repo.get());

	git_oid tree_id;
	GIT2_CALL("Could not write index to tree", 
		git_index_write_tree, &tree_id, repo_index.get());
	GIT2_CALL("Could not write index to disk", 
		git_index_write, repo_index.get());

	git_tree_ptr tree;
	GIT2_PTR("Could not lookup tree from ID", 
		git_tree_lookup, tree, repo.get(), &tree_id);

	git_signature_ptr default_sign;
	GIT2_PTR("Could not get default signature", 
		git_signature_default, default_sign, repo.get());

	git_oid parent_commit_id;
	GIT2_CALL("Could not get parent ID", 
		git_reference_name_to_id, &parent_commit_id, repo.get(), "HEAD");

	git_commit_ptr parent_commit;
	GIT2_PTR("Could not lookup parent commit data", 
		git_commit_lookup, parent_commit, repo.get(), &parent_commit_id);

	git_oid new_commit_id;
	if (!has_merge) {

		GIT2_CALL("Could not create commit",
			git_commit_create_v,
				&new_commit_id,
				repo.get(),
				"HEAD",
				default_sign.get(),
				default_sign.get(),
				"UTF-8",
				CString(msg).data,
				tree.get(),
				1,
				parent_commit.get());
	} else {
		git_commit_ptr fetchhead_commit;
		GIT2_PTR("Could not lookup commit pointed to by HEAD", 
			git_commit_lookup, fetchhead_commit, repo.get(), &pull_merge_oid);

		GIT2_CALL("Could not create commit",
			git_commit_create_v,
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
				fetchhead_commit.get());
		has_merge = false;
		GIT2_CALL("Could not clean repository state", 
			git_repository_state_cleanup, repo.get());
	}
}

void GitAPI::_stage_file(const String file_path) {
	CString c_path(file_path);
	char *paths[] = { c_path.data };
	git_strarray array = { paths, 1 };

	git_index_ptr index;
	GIT2_PTR("Could not get repository index", 
		git_repository_index, index, repo.get());
	GIT2_CALL("Could not add " + file_path + " to index", 
		git_index_add_all, index.get(), &array, GIT_INDEX_ADD_DEFAULT | GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, NULL, NULL);

	GIT2_CALL("Could not write changes to disk", 
		git_index_write, index.get());
}

void GitAPI::_unstage_file(const String file_path) {
	CString c_path(file_path);
	char *paths[] = { c_path.data };
	git_strarray array = { paths, 1 };

	git_reference_ptr head;
	GIT2_PTR("Could not get repository HEAD", 
		git_repository_head, head, repo.get());
	
	git_object_ptr head_commit;
	GIT2_PTR("Could not peel HEAD reference", 
		git_reference_peel, head_commit, head.get(), GIT_OBJ_COMMIT);

	GIT2_CALL("Could not reset " + file_path + " to state at HEAD", 
		git_reset_default, repo.get(), head_commit.get(), &array);
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
}

bool GitAPI::create_initial_commit() {
	git_signature_ptr sig;
	GIT2_PTR_R("Unable to create a commit signature. Perhaps 'user.name' and 'user.email' are not set. Set default user name and user email by `git config` and initialize again", 
		false,
		git_signature_default, sig, repo.get());

	git_index_ptr repo_index;
	GIT2_PTR_R("Could not get repository index", false, 
		git_repository_index, repo_index, repo.get());
	
	git_oid tree_id;
	GIT2_CALL_R("Could not write index to tree", false, 
		git_index_write_tree, &tree_id, repo_index.get());

	git_tree_ptr tree;
	GIT2_PTR_R("Could not lookup tree from disk", false, git_tree_lookup, tree, repo.get(), &tree_id);
	
	git_oid commit_id;
	GIT2_CALL_R("Could not create the initial commit", false,
		git_commit_create_v, 
			&commit_id, 
			repo.get(), 
			"HEAD", 
			sig.get(), 
			sig.get(), 
			NULL, 
			"Initial commit", 
			tree.get(), 
			0);

	GIT2_CALL_R("Could not write index to disk", false, 
		git_index_write, repo_index.get());

	return true;
}

bool GitAPI::_is_vcs_initialized() {
	return repo != nullptr;
}

Array GitAPI::_get_modified_files_data() {

	Array stats_files;

	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	opts.flags = GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
	opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

	git_status_list_ptr statuses;
	GIT2_PTR_R("Could not get status information from repository", Array(),
		git_status_list_new, statuses, repo.get(), &opts);

	size_t count = git_status_list_entrycount(statuses.get());
	for (size_t i = 0; i < count; ++i) {
		const git_status_entry *entry = git_status_byindex(statuses.get(), i);
		String path;
		if (entry->index_to_workdir) {
			path = entry->index_to_workdir->new_file.path;
		} else {
			path = entry->head_to_index->new_file.path;
		}

		static Dictionary map_changes;
		map_changes[GIT_STATUS_WT_NEW] = CHANGE_TYPE_NEW;
		map_changes[GIT_STATUS_INDEX_NEW] = CHANGE_TYPE_NEW;
		map_changes[GIT_STATUS_WT_MODIFIED] = CHANGE_TYPE_MODIFIED;
		map_changes[GIT_STATUS_INDEX_MODIFIED] = CHANGE_TYPE_MODIFIED;
		map_changes[GIT_STATUS_WT_RENAMED] = CHANGE_TYPE_RENAMED;
		map_changes[GIT_STATUS_INDEX_RENAMED] = CHANGE_TYPE_RENAMED;
		map_changes[GIT_STATUS_WT_DELETED] = CHANGE_TYPE_DELETED;
		map_changes[GIT_STATUS_INDEX_DELETED] = CHANGE_TYPE_DELETED;
		map_changes[GIT_STATUS_WT_TYPECHANGE] = CHANGE_TYPE_TYPECHANGE;
		map_changes[GIT_STATUS_INDEX_TYPECHANGE] = CHANGE_TYPE_TYPECHANGE;
		map_changes[GIT_STATUS_CONFLICTED] = CHANGE_TYPE_UNMERGED;

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
			}
			else{
				stats_files.push_back(create_status_file(path, map_changes[entry->status & git_status_index], TREE_AREA_STAGED));
			}
		}
	}

	return stats_files;
}

Array GitAPI::_get_branch_list() {

	git_branch_iterator_ptr it;
	GIT2_PTR_R("Could not create branch iterator", Array(),
		git_branch_iterator_new, it, repo.get(), GIT_BRANCH_LOCAL);

	Array branch_names;
	branch_names.push_back(String()); // Leave space for current branch

	git_reference *ref;
	git_branch_t type;
	while (git_branch_next(&ref, &type, it.get()) != GIT_ITEROVER) {
		const char *name;
		GIT2_CALL_R("Could not get branch name", Array(), 
			git_branch_name, &name, ref);
		if (git_branch_is_head(ref)) {
			branch_names[0] = String(name);
		} else {
			branch_names.push_back(String(name));
		}
		git_reference_free(ref);
	}

	return branch_names;
}

void GitAPI::_create_branch(const String branch_name) {
	git_oid head_commit_id;
	GIT2_CALL("Could not get HEAD commit ID", 
		git_reference_name_to_id, &head_commit_id, repo.get(), "HEAD");

	git_commit_ptr head_commit;
	GIT2_PTR("Could not lookup HEAD commit",
		git_commit_lookup, head_commit, repo.get(), &head_commit_id);

	git_reference_ptr branch_ref;
	GIT2_PTR("Could not create branch from HEAD",
		git_branch_create, branch_ref, repo.get(), CString(branch_name).data, head_commit.get(), 0);
}

void GitAPI::_create_remote(const String remote_name, const String remote_url) {
	git_remote_ptr remote;
	GIT2_PTR("Could not create remote",
		git_remote_create, remote, repo.get(), CString(remote_name).data, CString(remote_url).data);
}

Array GitAPI::_get_line_diff(String file_path, String text) {

	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;

	opts.context_lines = 0;
	opts.flags = GIT_DIFF_DISABLE_PATHSPEC_MATCH | GIT_DIFF_INCLUDE_UNTRACKED;

	git_index_ptr index;
	GIT2_PTR_R("Failed to get repository index", Array(), 
		git_repository_index, index, repo.get());
	GIT2_CALL_R("Failed to read index", Array(), 
		git_index_read, index.get(), 0);

	const git_index_entry *entry = git_index_get_bypath(index.get(), CString(file_path).data, GIT_INDEX_STAGE_NORMAL);
	
	if (entry == NULL) {
		return Array();
	}

	git_reference_ptr head;
	GIT2_PTR_R("Failed to load repository head", Array(),
		git_repository_head, head, repo.get());

	git_blob_ptr blob;
	GIT2_PTR_R("Failed to load head blob", Array(), 
		git_blob_lookup, blob, repo.get(), &entry->id);

	Array diff_contents;

	DiffHelper diff_helper = { &diff_contents, this };
	GIT2_CALL_R("Failed to make diff", Array(),
		git_diff_blob_to_buffer, blob.get(), NULL, CString(text).data, text.length(), NULL, &opts, NULL, NULL, diff_hunk_cb, NULL, &diff_helper);

	return diff_contents;
}

String GitAPI::_get_current_branch_name(bool full_ref) {

	git_reference_ptr head;
	GIT2_PTR_R("Could not get repository HEAD", "",
		git_repository_head, head, repo.get());

	git_reference_ptr branch;
	GIT2_PTR_R("Could not resolve HEAD reference", "", 
		git_reference_resolve, branch, head.get());

	String branch_name;
	if (full_ref) {
		branch_name = git_reference_name(branch.get());
	} else {
		const char *name = "";
		GIT2_CALL_R("Could not get branch name from current branch reference", "", 
			git_branch_name, &name, branch.get());
		branch_name = name;
	}

	return branch_name;
}

Array GitAPI::_get_remotes() {
	git_strarray remote_array;
	GIT2_CALL_R("Could not get list of remotes", Array(),
		git_remote_list, &remote_array, repo.get());

	Array remotes;
	for (int i = 0; i < remote_array.count; i++) {
		remotes.push_back(remote_array.strings[i]);
	}

	return remotes;
}

Array GitAPI::_get_previous_commits() {

	git_revwalk_ptr walker;

	GIT2_PTR_R("Could not create new revwalk", Array(),
		git_revwalk_new, walker, repo.get());
	GIT2_CALL_R("Could not sort revwalk by time", Array(), 
		git_revwalk_sorting, walker.get(), GIT_SORT_TIME);
	GIT2_CALL_R("Could not push head to revwalk", Array(), 
		git_revwalk_push_head, walker.get());

	git_oid oid;
	Array commits;
	char commit_id[GIT_OID_HEXSZ + 1];
	for (int i = 0; !git_revwalk_next(&oid, walker.get()) && i <= max_commit_fetch; i++) {
		git_commit_ptr commit;
		GIT2_PTR_R("Failed to lookup the commit", commits, 
			git_commit_lookup, commit, repo.get(), &oid);
		
		git_oid_tostr(commit_id, GIT_OID_HEXSZ + 1, git_commit_id(commit.get()));
		
		String msg = git_commit_message(commit.get());
		String author = git_commit_author(commit.get())->name;
		author = author + " <" + String(git_commit_author(commit.get())->email) + ">";
		int64_t when = git_commit_time(commit.get());
		int64_t offset = git_commit_time_offset(commit.get());
		String hex_id = commit_id;
		
		Dictionary commit_info = create_commit(msg, author, hex_id, String::num_int64(when), offset);
		
		commits.push_back(commit_info);
	}

	return commits;
}

void GitAPI::_fetch(String remote, String username, String password) {

	Godot::print("GitAPI: Performing fetch from " + remote);
	
	git_remote_ptr remote_object;
	GIT2_PTR("Could not lookup remote \"" + remote + "\"",
		git_remote_lookup, remote_object, repo.get(), CString(remote).data);

	CString c_username(username);
	CString c_password(username);

	String payload[2] = {
		username,
		password
	};

	git_remote_callbacks remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = &credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = &transfer_progress_cb;
	remote_cbs.payload = payload;
	remote_cbs.push_transfer_progress = &push_transfer_progress_cb;
	remote_cbs.push_update_reference = &push_update_reference_cb;

	GIT2_CALL("Could not connect to remote \"" + remote + "\". Are your credentials correct? Try using a PAT token (in case you are using Github) as your password",
		git_remote_connect, remote_object.get(), GIT_DIRECTION_FETCH, &remote_cbs, NULL, NULL);

	git_fetch_options opts = GIT_FETCH_OPTIONS_INIT;
	opts.callbacks = remote_cbs;
	GIT2_CALL("Could not fetch data from remote", 
		git_remote_fetch, remote_object.get(), NULL, &opts, "fetch");

	Godot::print("GitAPI: Fetch ended");
}

void GitAPI::_pull(String remote, String username, String password) {
	Godot::print("GitAPI: Performing pull from " + remote);

	git_remote_ptr remote_object;
	GIT2_PTR("Could not lookup remote \"" + remote + "\"",
		git_remote_lookup, remote_object, repo.get(), CString(remote).data);

	String payload[2] = {
		username,
		password
	};

	git_remote_callbacks remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = &credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = &transfer_progress_cb;
	remote_cbs.payload = payload;
	remote_cbs.push_transfer_progress = &push_transfer_progress_cb;
	remote_cbs.push_update_reference = &push_update_reference_cb;

	GIT2_CALL("Could not connect to remote \"" + remote + "\". Are your credentials correct? Try using a PAT token (in case you are using Github) as your password",
		git_remote_connect, remote_object.get(), GIT_DIRECTION_FETCH, &remote_cbs, NULL, NULL);

	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	fetch_opts.callbacks = remote_cbs;

	String branch_name = _get_current_branch_name(true);

	CString ref_spec_str(branch_name);
	
	char *ref[] = { ref_spec_str.data };
	git_strarray refspec = { ref, 1 };

	GIT2_CALL("Could not fetch data from remote", 
		git_remote_fetch, remote_object.get(), &refspec, &fetch_opts, "pull");
	
	pull_merge_oid = {};
	GIT2_CALL("Could not read \"FETCH_HEAD\" file", 
		git_repository_fetchhead_foreach, repo.get(), fetchhead_foreach_cb, &pull_merge_oid);

	if (git_oid_is_zero(&pull_merge_oid)) {
		popup_error("GitAPI: Could not find remote branch HEAD for " + branch_name + ". Try pushing the branch first.");
		return;
	}

	git_annotated_commit_ptr fetchhead_annotated_commit;
	GIT2_PTR("Could not get merge commit",
		git_annotated_commit_lookup, fetchhead_annotated_commit, repo.get(), &pull_merge_oid);

	const git_annotated_commit *merge_heads[] = { fetchhead_annotated_commit.get() };

	git_merge_analysis_t merge_analysis;
	git_merge_preference_t preference = GIT_MERGE_PREFERENCE_NONE;
	GIT2_CALL("Merge analysis failed", 
		git_merge_analysis, &merge_analysis, &preference, repo.get(), merge_heads, 1);

	if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
		git_checkout_options ff_checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
		int err = 0;

		git_reference_ptr target_ref;
		GIT2_PTR("Failed to get HEAD reference", 
			git_repository_head, target_ref, repo.get());

		git_object_ptr target;
		GIT2_PTR("Failed to lookup OID " + String(git_oid_tostr_s(&pull_merge_oid)),
			git_object_lookup, target, repo.get(), &pull_merge_oid, GIT_OBJECT_COMMIT);

		ff_checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
		GIT2_CALL("Failed to checkout HEAD reference", 
			git_checkout_tree, repo.get(), target.get(), &ff_checkout_options);

		git_reference_ptr new_target_ref;
		GIT2_PTR("Failed to move HEAD reference", 
			git_reference_set_target, new_target_ref, target_ref.get(), &pull_merge_oid, NULL);

		Godot::print("GitAPI: Fast Forwarded");
		GIT2_CALL("Could not clean repository state", 
			git_repository_state_cleanup, repo.get());

	} else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
		git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
		git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

		merge_opts.file_favor = GIT_MERGE_FILE_FAVOR_NORMAL;
		merge_opts.file_flags = (GIT_MERGE_FILE_STYLE_DIFF3 | GIT_MERGE_FILE_DIFF_MINIMAL);
		checkout_opts.checkout_strategy = (GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS | GIT_CHECKOUT_CONFLICT_STYLE_MERGE);
		GIT2_CALL("Merge Failed", 
			git_merge, repo.get(), merge_heads, 1, &merge_opts, &checkout_opts);

		git_index_ptr index;
		GIT2_PTR("Could not get repository index", 
			git_repository_index, index, repo.get());

		if (git_index_has_conflicts(index.get())) {
			popup_error("GitAPI: Index has conflicts, Solve conflicts and make a merge commit.");
		} else {
			popup_error("GitAPI: Change are staged, make a merge commit.");
		}

		has_merge = true;

	} else if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
		Godot::print("GitAPI: Already up to date");

		GIT2_CALL("Could not clean repository state", 
			git_repository_state_cleanup, repo.get());

	} else {
		Godot::print("GitAPI: Can not merge");
	}

	Godot::print("GitAPI: Pull ended");
}

void GitAPI::_push(const String remote, const String username, const String password, const bool force) {
	Godot::print("GitAPI: Performing push to " + remote);

	git_remote_ptr remote_object;
	GIT2_PTR("Could not lookup remote \"" + remote + "\"", 
		git_remote_lookup, remote_object, repo.get(), CString(remote).data);

	CString c_username(username);
	CString c_password(username);

	String payload[2] = {
		username,
		password
	};

	git_remote_callbacks remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = &credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = &transfer_progress_cb;
	remote_cbs.payload = payload;
	remote_cbs.push_transfer_progress = &push_transfer_progress_cb;
	remote_cbs.push_update_reference = &push_update_reference_cb;

	GIT2_CALL("Could not connect to remote \"" + remote + "\". Are your credentials correct? Try using a PAT token (in case you are using Github) as your password", 
		git_remote_connect, remote_object.get(), GIT_DIRECTION_PUSH, &remote_cbs, NULL, NULL);

	String branch_name = _get_current_branch_name(true);

	CString pushspec(String() + (force ? "+" : "") + branch_name);
	const git_strarray refspec = { &pushspec.data, 1 };

	git_push_options push_options = GIT_PUSH_OPTIONS_INIT;
	push_options.callbacks = remote_cbs;

	GIT2_CALL("Failed to push",
		git_remote_push, remote_object.get(), &refspec, &push_options);
	
	Godot::print("GitAPI: Push ended");
}

bool GitAPI::_checkout_branch(String branch_name) {

	git_reference_ptr branch;
	GIT2_PTR_R("Could not find branch", false,
		git_branch_lookup, branch, repo.get(), CString(branch_name).data, GIT_BRANCH_LOCAL);
	const char *branch_ref_name = git_reference_name(branch.get());

	git_object_ptr treeish;
	GIT2_PTR_R("Could not find branch head", false, 
		git_revparse_single, treeish, repo.get(), CString(branch_name).data);

	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	GIT2_CALL_R("Could not checkout branch tree", false,
		git_checkout_tree, repo.get(), treeish.get(), &opts);
	GIT2_CALL_R("Could not set head", false, 
		git_repository_set_head, repo.get(), branch_ref_name);

	return true;
}

Array GitAPI::_get_file_diff(const String identifier, const int64_t area) {
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
			GIT2_PTR_R("Could not create diff for index from working directory", diff_contents,
				git_diff_index_to_workdir, diff, repo.get(), NULL, &opts);
		} break;
		case TREE_AREA_STAGED: {
			git_object_ptr obj;
			GIT2_PTR_R("Could not get HEAD^{tree} object", diff_contents,
				git_revparse_single, obj, repo.get(), "HEAD^{tree}");
			
			git_tree_ptr tree;
			GIT2_PTR_R("Could not lookup HEAD^{tree}", diff_contents,
				git_tree_lookup, tree, repo.get(), git_object_id(obj.get()));

			GIT2_PTR_R("Could not create diff for tree from index directory", diff_contents,
				git_diff_tree_to_index, diff, repo.get(), tree.get(), NULL, &opts);
		} break;
		case TREE_AREA_COMMIT: {

			opts.pathspec = {};
			
			git_object_ptr obj;
			GIT2_PTR_R("Could not get object at " + identifier, diff_contents, 
				git_revparse_single, obj, repo.get(), pathspec.data);
			
			git_commit_ptr commit;
			git_commit_ptr parent;
			GIT2_PTR_R("Could not get commit "+ identifier, diff_contents,
				git_commit_lookup, commit, repo.get(), git_object_id(obj.get()));
			GIT2_PTR_R("Could not get parent commit of " + identifier, diff_contents,
				git_commit_parent, parent, commit.get(), 0);

			git_tree_ptr commit_tree;
			git_tree_ptr parent_tree;
			GIT2_PTR_R("Could not get commit tree of " + identifier, diff_contents,
				git_commit_tree, commit_tree, commit.get());
			GIT2_PTR_R("Could not get parent commit tree of " + identifier, diff_contents, 
				git_commit_tree, parent_tree, parent.get());
			
			GIT2_PTR_R("Could not generate diff for commit " + identifier, diff_contents, 
				git_diff_tree_to_tree, diff, repo.get(), parent_tree.get(), commit_tree.get(), &opts);

		} break;
	}

	diff_contents = _parse_diff(diff.get());

	return diff_contents;
}

Array GitAPI::_parse_diff(git_diff *diff) {

	Array diff_contents;
	for (int i = 0; i < git_diff_num_deltas(diff); i++) {
		const git_diff_delta *delta = git_diff_get_delta(diff, i); //file_cb
		
		git_patch_ptr patch;
		GIT2_PTR_R("Could not create patch from diff", Array(),
			git_patch_from_diff, patch, diff, i);

		if (delta->status == GIT_DELTA_UNMODIFIED || delta->status == GIT_DELTA_IGNORED) {
			continue;
		}

		Dictionary diff_file = create_diff_file(delta->new_file.path, delta->old_file.path);

		Array diff_hunks;
		for (int j = 0; j < git_patch_num_hunks(patch.get()); j++) {
			const git_diff_hunk *git_hunk;
			size_t line_count;
			GIT2_CALL_R("Could not get hunk from patch", Array(),
				git_patch_get_hunk, &git_hunk, &line_count, patch.get(), j);

			Dictionary diff_hunk = create_diff_hunk(git_hunk->old_start, git_hunk->new_start, git_hunk->old_lines, git_hunk->new_lines);

			Array diff_lines;
			for (int k = 0; k < line_count; k++) {
				const git_diff_line *git_diff_line;
				GIT2_CALL_R("Could not get line from hunk in patch", Array(), 
					git_patch_get_line_in_hunk, &git_diff_line, patch.get(), j, k); // line_cb
				
				char *content = new char[git_diff_line->content_len + 1];
				memcpy(content, git_diff_line->content, git_diff_line->content_len);
				content[git_diff_line->content_len] = '\0';

				Dictionary diff_line = create_diff_line(git_diff_line->new_lineno, git_diff_line->old_lineno, String(content), String(git_diff_line->origin));
				diff_lines.push_back(diff_line);
			}

			diff_hunk = add_line_diffs_into_diff_hunk(diff_hunk, diff_lines);
			diff_hunks.push_back(diff_hunk);
		}
		diff_file = add_diff_hunks_into_diff_file(diff_file, diff_hunks);
		diff_contents.push_back(diff_file);
	}
	return diff_contents;
}

String GitAPI::_get_project_name() {
	return String("project");
}

String GitAPI::_get_vcs_name() {
	return "Git";
}

bool GitAPI::_initialize(String project_root_path) {
	ERR_FAIL_COND_V(project_root_path == "", false);

	int init = git_libgit2_init();
	if (init > 1) {
		WARN_PRINT("Multiple libgit2 instances are running");
	}

	GIT2_PTR_R("Could not initialize repository", false,
		git_repository_init, repo, CString(project_root_path).data, 0);
	
	git_reference* head = nullptr;
	int error = git_repository_head(&head, repo.get());
	if (error == GIT_EUNBORNBRANCH || error == GIT_ENOTFOUND)
	{
		create_gitignore_and_gitattributes();
		if (!create_initial_commit()) {
			popup_error("GitAPI: Initial commit could not be created. Commit functionality will not work.");
		}
	}
	else
	{
		check_errors(error, "Could not get repository HEAD", __FUNCTION__, __FILE__, __LINE__);
	}

	GIT2_PTR_R("Could not open repository", false,
		git_repository_open, repo, CString(project_root_path).data);

	return true;
}

bool GitAPI::_shut_down() {
	repo.reset(); // Destroy repo object before libgit2 shuts down
	GIT2_CALL_R("Could not shutdown Git Plugin", false, 
		git_libgit2_shutdown);
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
