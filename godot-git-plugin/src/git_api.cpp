#include "git_api.h"

namespace godot {

void GitAPI::_register_methods() {

	register_method("_process", &GitAPI::_process);

	register_method("_commit", &GitAPI::_commit);
	register_method("_is_vcs_initialized", &GitAPI::_is_vcs_initialized);
	register_method("_get_modified_files_data", &GitAPI::_get_modified_files_data);
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
	register_method("_checkout_branch", &GitAPI::_checkout_branch);
	register_method("_fetch", &GitAPI::_fetch);
	register_method("_pull", &GitAPI::_pull);
	register_method("_push", &GitAPI::_push);
	register_method("_set_up_credentials", &GitAPI::_set_up_credentials);
	register_method("_get_line_diff", &GitAPI::_get_line_diff);
}

void GitAPI::_discard_file(String p_file_path) {

	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	char *paths[] = { p_file_path.alloc_c_string() };
	opts.paths = { paths, 1 };
	opts.checkout_strategy = GIT_CHECKOUT_FORCE;
	GIT2_CALL(git_checkout_index(repo, NULL, &opts), "Cannot checkout a file");
}

void GitAPI::_commit(const String p_msg) {

	if (!can_commit) {
		godot::Godot::print("Git API: Cannot commit. Check previous errors.");
		return;
	}

	git_signature *default_sign;
	git_oid tree_id, parent_commit_id, new_commit_id;
	git_tree *tree;
	git_index *repo_index;
	git_commit *parent_commit;

	GIT2_CALL(git_repository_index(&repo_index, repo), "Could not get repository index");
	GIT2_CALL(git_index_write_tree(&tree_id, repo_index), "Could not write index to tree");
	GIT2_CALL(git_index_write(repo_index), "Could not write index to disk");

	GIT2_CALL(git_tree_lookup(&tree, repo, &tree_id), "Could not lookup tree from ID");
	GIT2_CALL(git_signature_default(&default_sign, repo), "Could not get default signature");

	GIT2_CALL(git_reference_name_to_id(&parent_commit_id, repo, "HEAD"), "Could not get parent ID");
	GIT2_CALL(git_commit_lookup(&parent_commit, repo, &parent_commit_id), "Could not lookup parent commit data");

	if (!has_merge) {

		GIT2_CALL(
				git_commit_create_v(
						&new_commit_id,
						repo,
						"HEAD",
						default_sign,
						default_sign,
						"UTF-8",
						p_msg.alloc_c_string(),
						tree,
						1,
						parent_commit),
				"Could not create commit");
	} else {
		git_commit *fetchhead_commit;
		git_commit_lookup(&fetchhead_commit, repo, &pull_merge_oid);

		GIT2_CALL(
				git_commit_create_v(
						&new_commit_id,
						repo,
						"HEAD",
						default_sign,
						default_sign,
						"UTF-8",
						p_msg.alloc_c_string(),
						tree,
						2,
						parent_commit,
						fetchhead_commit),
				"Could not create commit");
		has_merge = false;
		git_commit_free(fetchhead_commit);
		git_repository_state_cleanup(repo);
	}

	git_index_free(repo_index);
	git_signature_free(default_sign);
	git_commit_free(parent_commit);
	git_tree_free(tree);
}

void GitAPI::_stage_file(const String p_file_path) {
	git_index *index;
	char *paths[] = { p_file_path.alloc_c_string() };
	git_strarray array = { paths, 1 };

	GIT2_CALL(git_repository_index(&index, repo), "Could not get repository index");
	GIT2_CALL(git_index_add_all(index, &array, GIT_INDEX_ADD_DEFAULT | GIT_INDEX_ADD_DISABLE_PATHSPEC_MATCH, NULL, NULL), "Could not add a file");

	GIT2_CALL(git_index_write(index), "Could not write changes to disk");
	git_index_free(index);
}

void GitAPI::_unstage_file(const String p_file_path) {

	char *paths[] = { p_file_path.alloc_c_string() };
	git_strarray array = { paths, 1 };

	git_reference *head;
	git_object *head_commit;

	git_repository_head(&head, repo);
	git_reference_peel(&head_commit, head, GIT_OBJ_COMMIT);

	git_reset_default(repo, head_commit, &array);
}

void GitAPI::create_gitignore_and_gitattributes() {

	File *file = File::_new();

	if (!file->file_exists("res://.gitignore")) {

		file->open("res://.gitignore", File::ModeFlags::WRITE);
		file->store_string(
				"# Import cache\n"
				".import/\n\n"

				"# Binaries\n"
				"bin/\n"
				"build/\n"
				"lib/\n");
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

	git_signature *sig;
	git_oid tree_id, commit_id;
	git_index *repo_index;
	git_tree *tree;

	if (git_signature_default(&sig, repo) != 0) {

		godot::Godot::print_error("Unable to create a commit signature. Perhaps 'user.name' and 'user.email' are not set. Set default user name and user email by `git config` and initialize again", __func__, __FILE__, __LINE__);
		return false;
	}
	GIT2_CALL_R(git_repository_index(&repo_index, repo), "Could not get repository index", false);
	GIT2_CALL_R(git_index_write_tree(&tree_id, repo_index), "Could not create intial commit", false);

	GIT2_CALL_R(git_tree_lookup(&tree, repo, &tree_id), "Could not create intial commit", false);
	GIT2_CALL_R(
			git_commit_create_v(&commit_id, repo, "HEAD", sig, sig, NULL, "Initial commit", tree, 0),
			"Could not create the initial commit",
			false);

	GIT2_CALL_R(git_index_write(repo_index), "Could not write index to disk", false);
	git_index_free(repo_index);
	git_tree_free(tree);
	git_signature_free(sig);

	return true;
}

bool GitAPI::_is_vcs_initialized() {

	return is_initialized;
}

Array GitAPI::_get_modified_files_data() {

	Array stats_files;

	git_status_options opts = GIT_STATUS_OPTIONS_INIT;
	opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
	opts.flags = GIT_STATUS_OPT_EXCLUDE_SUBMODULES;
	opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

	git_status_list *statuses = NULL;
	GIT2_CALL_R(git_status_list_new(&statuses, repo, &opts), "Could not get status information from repository", stats_files);

	size_t count = git_status_list_entrycount(statuses);
	for (size_t i = 0; i < count; ++i) {

		const git_status_entry *entry = git_status_byindex(statuses, i);
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
			stats_files.push_back(_create_status_file(path, map_changes[entry->status & git_status_wt], TREE_AREA_UNSTAGED));
		}
		
		if (entry->status & git_status_index) {
			if (entry->status == GIT_STATUS_INDEX_RENAMED) {
				String old_path = entry->head_to_index->old_file.path;
				stats_files.push_back(_create_status_file(old_path, map_changes[GIT_STATUS_INDEX_DELETED], TREE_AREA_STAGED));			
				stats_files.push_back(_create_status_file(path, map_changes[GIT_STATUS_INDEX_NEW], TREE_AREA_STAGED));
			}
			else{
				stats_files.push_back(_create_status_file(path, map_changes[entry->status & git_status_index], TREE_AREA_STAGED));
			}
		}
	}

	git_status_list_free(statuses);
	return stats_files;
}

Array GitAPI::_get_branch_list() {
	git_branch_iterator *it;
	git_reference *ref;
	git_branch_t type;
	Array branch_names;
	git_branch_iterator_new(&it, repo, GIT_BRANCH_LOCAL);

	branch_names.push_back(String()); // Leave Space for current branch

	while (git_branch_next(&ref, &type, it) != GIT_ITEROVER) {
		const char *name;
		git_branch_name(&name, ref);
		if (git_branch_is_head(ref)) {
			branch_names[0] = String(name);
		} else {
			branch_names.push_back(String(name));
		}
		git_reference_free(ref);
	}
	git_branch_iterator_free(it);

	return branch_names;
}

Array GitAPI::_get_line_diff(String p_file_path, String p_text) {
	//get blob
	git_index *index;
	git_blob *blob;
	git_reference *head;

	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_diff *diff;
	Array diff_contents;

	opts.context_lines = 0;
	opts.flags = GIT_DIFF_DISABLE_PATHSPEC_MATCH | GIT_DIFF_INCLUDE_UNTRACKED;

	GIT2_CALL_R(git_repository_index(&index, repo), "Failed to get repository index", diff_contents);
	GIT2_CALL_R(git_index_read(index, 0), "Failed to read index", diff_contents);

	const git_index_entry *entry = git_index_get_bypath(index, p_file_path.alloc_c_string(), GIT_INDEX_STAGE_NORMAL);
	
	if (entry == NULL) {
		git_index_free(index);
		return diff_contents;
	}

	const git_oid *blobSha = &entry->id;
	GIT2_CALL_R(git_repository_head(&head, repo), "Failed to load repository head", diff_contents);
	GIT2_CALL_R(git_blob_lookup(&blob, repo, blobSha), "Failed to load head blob", diff_contents);
	GIT2_CALL_R(git_diff_blob_to_buffer(blob, NULL, p_text.alloc_c_string(), p_text.length(), NULL, &opts, NULL, NULL, diff_hunk_cb, NULL, &diff_contents), "Failed to make diff", diff_contents);

	git_index_free(index);
	git_blob_free(blob);
	git_reference_free(head);
	git_diff_free(diff);

	return diff_contents;
}

const char *GitAPI::_get_current_branch_name(bool full_ref) {
	git_reference *head, *branch;
	git_repository_head(&head, repo);
	git_reference_resolve(&branch, head);
	const char *branch_name;
	if (full_ref) {
		branch_name = git_reference_name(branch);
	} else {
		git_branch_name(&branch_name, branch);
	}

	git_reference_free(head);
	git_reference_free(branch);
	return branch_name;
}

Array GitAPI::_get_previous_commits() {

	Array commits;
	git_revwalk *walker;
	git_commit *commit;
	const git_signature *sig;
	git_oid oid;
	char commit_id[GIT_OID_HEXSZ + 1];

	git_revwalk_new(&walker, repo);
	git_revwalk_sorting(walker, GIT_SORT_TIME);
	git_revwalk_push_head(walker);

	for (int i = 0; !git_revwalk_next(&oid, walker) && i <= max_commit_fetch; i++) {
		GIT2_CALL_R(git_commit_lookup(&commit, repo, &oid), "Failed to lookup the commit", commits);
		sig = git_commit_author(commit);
		git_oid_tostr(commit_id, GIT_OID_HEXSZ + 1, git_commit_id(commit));
		String msg = git_commit_message(commit);
		String author = sig->name;
		int64_t when = (int64_t)sig->when.time + (int64_t)(sig->when.offset * 60);
		String hex_id = commit_id;
		Dictionary commit_info = _create_commit(msg, author, hex_id, when);
		commits.push_back(commit_info);
		git_commit_free(commit);
	}

	git_revwalk_free(walker);

	return commits;
}

void GitAPI::_fetch() {

	Godot::print("Git API: Performing fetch...");
	GIT2_CALL(git_remote_connect(remote, GIT_DIRECTION_FETCH, &remote_cbs, NULL, NULL), "Can not connect to remote \"" + String(remote_name) + "\"");

	git_fetch_options opts = GIT_FETCH_OPTIONS_INIT;
	opts.callbacks = remote_cbs;
	GIT2_CALL(git_remote_fetch(remote, NULL, &opts, "fetch"), "Can not fetch data from remote");
}

void GitAPI::_pull() {
	Godot::print("Git API: Performing pull...");

	GIT2_CALL(git_remote_connect(remote, GIT_DIRECTION_FETCH, &remote_cbs, NULL, NULL), "Can not connect to remote \"" + String(remote_name) + "\"");

	git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
	fetch_opts.callbacks = remote_cbs;

	const char *branch_name = _get_current_branch_name(false);
	// There is no way to get remote branch name from current branch name with libgit2
	String ref_name = "refs/heads/" + String(branch_name) + ":refs/remotes/" + String(remote_name) + "/" + String(branch_name);
	char *ref[] = { ref_name.alloc_c_string() };
	git_strarray refspec = { ref, 1 };

	GIT2_CALL(git_remote_fetch(remote, &refspec, &fetch_opts, "pull"), "Can not fetch data from remote");
	GIT2_CALL(git_repository_fetchhead_foreach(repo, fetchhead_foreach_cb, &pull_merge_oid), "Can not read \"FETCH_HEAD\" file");

	if (!&pull_merge_oid) {
		return;
	}

	git_annotated_commit *fetchhead_annotated_commit;
	GIT2_CALL(git_annotated_commit_lookup(&fetchhead_annotated_commit, repo, &pull_merge_oid), "Can not get merge commit");

	const git_annotated_commit *merge_heads[] = { fetchhead_annotated_commit };

	git_merge_analysis_t merge_analysis;
	git_merge_preference_t preference = GIT_MERGE_PREFERENCE_NONE;
	GIT2_CALL(git_merge_analysis(&merge_analysis, &preference, repo, merge_heads, 1), "Merge analysis failed");

	if (merge_analysis & GIT_MERGE_ANALYSIS_FASTFORWARD) {
		git_checkout_options ff_checkout_options = GIT_CHECKOUT_OPTIONS_INIT;
		git_reference *target_ref;
		git_reference *new_target_ref;
		git_object *target = NULL;
		int err = 0;

		GIT2_CALL(git_repository_head(&target_ref, repo), "Failed to get HEAD reference");
		GIT2_CALL(git_object_lookup(&target, repo, &pull_merge_oid, GIT_OBJECT_COMMIT), "failed to lookup OID " + String(git_oid_tostr_s(&pull_merge_oid)));

		ff_checkout_options.checkout_strategy = GIT_CHECKOUT_SAFE;
		GIT2_CALL(git_checkout_tree(repo, target, &ff_checkout_options), "Failed to checkout HEAD reference");

		GIT2_CALL(git_reference_set_target(&new_target_ref, target_ref, &pull_merge_oid, NULL), "Failed to move HEAD reference");

		git_reference_free(target_ref);
		git_reference_free(new_target_ref);
		Godot::print("Git API: Fast Forwarded");
		git_repository_state_cleanup(repo);
	} else if (merge_analysis & GIT_MERGE_ANALYSIS_NORMAL) {
		git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
		git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

		merge_opts.file_favor = GIT_MERGE_FILE_FAVOR_NORMAL;
		merge_opts.file_flags = (GIT_MERGE_FILE_STYLE_DIFF3 | GIT_MERGE_FILE_DIFF_MINIMAL);
		checkout_opts.checkout_strategy = (GIT_CHECKOUT_SAFE | GIT_CHECKOUT_ALLOW_CONFLICTS | GIT_CHECKOUT_CONFLICT_STYLE_DIFF3);
		GIT2_CALL(git_merge(repo, merge_heads, 1, &merge_opts, &checkout_opts), "Merge Failed");

		git_index *index;
		git_repository_index(&index, repo);

		if (git_index_has_conflicts(index)) {
			Godot::print("Git API: Index has conflicts, Solve conflicts and make a merge commit.");
		} else {
			Godot::print("Git API: Change are staged, make a merge commit.");
		}

		git_index_free(index);
		has_merge = true;

	} else if (merge_analysis & GIT_MERGE_ANALYSIS_UP_TO_DATE) {
		Godot::print("Git API: Already up to date");
		git_repository_state_cleanup(repo);
	} else {
		Godot::print("Git API: Can not merge");
	}

	git_annotated_commit_free(fetchhead_annotated_commit);
}

void GitAPI::_push() {
	Godot::print("Git API: Performing push...");

	GIT2_CALL(git_remote_connect(remote, GIT_DIRECTION_PUSH, &remote_cbs, NULL, NULL), "Can not connect to remote \"" + String(remote_name) + "\"");

	git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
	push_opts.callbacks = remote_cbs;

	String branch_name = String(_get_current_branch_name(true));
	char *ref[] = { branch_name.alloc_c_string() };
	git_strarray refspecs = { ref, 1 };

	GIT2_CALL(git_remote_upload(remote, &refspecs, &push_opts), "Failed to push");
}

void GitAPI::_set_up_credentials(String p_username, String p_password) {
	creds.username = p_username.alloc_c_string();
	creds.password = p_password.alloc_c_string();
}

bool GitAPI::_checkout_branch(String p_branch_name) {
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	opts.checkout_strategy = GIT_CHECKOUT_SAFE;
	git_reference *ref, *branch;
	git_object *treeish;

	GIT2_CALL_R(git_branch_lookup(&branch, repo, p_branch_name.alloc_c_string(), GIT_BRANCH_LOCAL), "", false);
	const char *branch_ref_name = git_reference_name(branch);

	GIT2_CALL_R(git_revparse_single(&treeish, repo, p_branch_name.alloc_c_string()), "", false);
	GIT2_CALL_R(git_checkout_tree(repo, treeish, &opts), "", false);
	GIT2_CALL_R(git_repository_set_head(repo, branch_ref_name), "", false);

	return true;
}

Array GitAPI::_get_file_diff(const String identifier, int area) {

	git_diff_options opts = GIT_DIFF_OPTIONS_INIT;
	git_diff *diff;
	Array diff_contents;

	opts.context_lines = 2;
	opts.interhunk_lines = 0;
	opts.flags = GIT_DIFF_DISABLE_PATHSPEC_MATCH | GIT_DIFF_INCLUDE_UNTRACKED;

	char *pathspec = identifier.alloc_c_string();
	opts.pathspec.strings = &pathspec;
	opts.pathspec.count = 1;

	switch ((TreeArea)area) {
		case TREE_AREA_UNSTAGED: {
			GIT2_CALL_R(git_diff_index_to_workdir(&diff, repo, NULL, &opts), "Could not create diff for index from working directory", diff_contents);
		} break;
		case TREE_AREA_STAGED: {
			git_object *obj = nullptr;
			git_tree *tree = nullptr;

			GIT2_CALL_R(git_revparse_single(&obj, repo, "HEAD^{tree}"), "", diff_contents);
			GIT2_CALL_R(git_tree_lookup(&tree, repo, git_object_id(obj)), "", diff_contents);
			GIT2_CALL_R(git_diff_tree_to_index(&diff, repo, tree, NULL, &opts), "Could not create diff for tree from index directory", diff_contents);

			git_tree_free(tree);
			git_object_free(obj);
		} break;
		case TREE_AREA_COMMIT: {
			opts.pathspec = {};
			git_object *obj = nullptr;
			git_commit *commit = nullptr, *parent = nullptr;
			git_tree *commit_tree = nullptr, *parent_tree = nullptr;

			GIT2_CALL_R(git_revparse_single(&obj, repo, pathspec), "", diff_contents);
			GIT2_CALL_R(git_commit_lookup(&commit, repo, git_object_id(obj)), "", diff_contents);
			GIT2_CALL_R(git_commit_parent(&parent, commit, 0), "", diff_contents);
			GIT2_CALL_R(git_commit_tree(&commit_tree, commit), "", diff_contents);
			GIT2_CALL_R(git_commit_tree(&parent_tree, parent), "", diff_contents);
			GIT2_CALL_R(git_diff_tree_to_tree(&diff, repo, parent_tree, commit_tree, &opts), "", diff_contents);

			git_object_free(obj);
			git_commit_free(commit);
			git_commit_free(parent);
			git_tree_free(commit_tree);
			git_tree_free(parent_tree);
		} break;
	}

	diff_contents = _parse_diff(diff);
	git_diff_free(diff);

	return diff_contents;
}

Array GitAPI::_parse_diff(git_diff *diff) {

	Array diff_contents;
	for (int i = 0; i < git_diff_num_deltas(diff); i++) {
		git_patch *patch;
		const git_diff_delta *delta = git_diff_get_delta(diff, i); //file_cb
		git_patch_from_diff(&patch, diff, i);

		if (delta->flags & GIT_DIFF_FLAG_BINARY) {
			continue;
		}

		if (delta->status == GIT_DELTA_UNMODIFIED || delta->status == GIT_DELTA_IGNORED || delta->status == GIT_DELTA_UNTRACKED) {
			continue;
		}
		Dictionary diff_file = _create_diff_file(delta->new_file.path, delta->old_file.path);

		Array diff_hunks;
		for (int j = 0; j < git_patch_num_hunks(patch); j++) {
			const git_diff_hunk *git_hunk;
			size_t line_count;
			git_patch_get_hunk(&git_hunk, &line_count, patch, j);

			Dictionary diff_hunk = _create_diff_hunk(git_hunk->old_start, git_hunk->new_start, git_hunk->old_lines, git_hunk->new_lines);

			Array diff_lines;
			for (int k = 0; k < line_count; k++) {
				const git_diff_line *git_diff_line;
				git_patch_get_line_in_hunk(&git_diff_line, patch, j, k); //line_cb
				char *content = new char[git_diff_line->content_len + 1];
				memcpy(content, git_diff_line->content, git_diff_line->content_len);
				content[git_diff_line->content_len] = '\0';

				Dictionary diff_line = _create_diff_line(git_diff_line->new_lineno, git_diff_line->old_lineno, String(content), String(git_diff_line->origin));
				diff_lines.push_back(diff_line);
			}

			diff_hunk = _add_line_diffs_into_diff_hunk(diff_hunk, diff_lines);
			diff_hunks.push_back(diff_hunk);
		}
		diff_file = _add_diff_hunks_into_diff_file(diff_file, diff_hunks);
		diff_contents.push_back(diff_file);
		
		git_patch_free(patch);
	}
	return diff_contents;
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

	if (repo) {

		return true;
	}

	can_commit = true;
	GIT2_CALL_R(git_repository_init(&repo, p_project_root_path.alloc_c_string(), 0), "Could not initialize repository", false);
	if (git_repository_head_unborn(repo) == 1) {

		create_gitignore_and_gitattributes();
		if (!create_initial_commit()) {

			godot::Godot::print_error("Initial commit could not be created. Commit functionality will not work.", __func__, __FILE__, __LINE__);
			can_commit = false;
		}
	}

	GIT2_CALL_R(git_repository_open(&repo, p_project_root_path.alloc_c_string()), "Could not open repository", false);
	GIT2_CALL_R(git_remote_lookup(&remote, repo, remote_name), "Can not find remote \"" + String(remote_name) + "\" ", true);

	remote_cbs = GIT_REMOTE_CALLBACKS_INIT;
	remote_cbs.credentials = credentials_cb;
	remote_cbs.update_tips = &update_cb;
	remote_cbs.sideband_progress = &progress_cb;
	remote_cbs.transfer_progress = transfer_progress_cb;
	remote_cbs.payload = &creds;
	remote_cbs.push_transfer_progress = push_transfer_progress_cb;
	remote_cbs.push_update_reference = push_update_reference_cb;

	is_initialized = true;

	return is_initialized;
}

bool GitAPI::_shut_down() {

	git_repository_free(repo);
	git_remote_free(remote);
	GIT2_CALL_R(git_libgit2_shutdown(), "Could not shutdown Git Addon", false);

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
