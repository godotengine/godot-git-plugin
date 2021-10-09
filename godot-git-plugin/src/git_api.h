#ifndef GIT_API_H
#define GIT_API_H

#include <memory>

#include <Button.hpp>
#include <Control.hpp>
#include <Directory.hpp>
#include <EditorVCSInterface.hpp>
#include <File.hpp>
#include <Godot.hpp>
#include <GodotGlobal.hpp>
#include <OS.hpp>
#include <AcceptDialog.hpp>
#include <LineEdit.hpp>

#include <allocation_defs.h>
#include <git_common.h>

#include <git2.h>

namespace godot {

class GitAPI;

struct DiffHelper {
	Array *diff_hunks;
	GitAPI *git_api;
};

struct CString {
	char *data = nullptr;

	CString() = delete;
	CString(const String& string) : data(string.alloc_c_string()) {}
	CString(CString &&) = delete;
	CString &operator=(const CString &) = delete;
	CString &operator=(CString &&) = delete;

	~CString() {
		if (data) {
			godot::api->godot_free(data);
			data = nullptr;
		}
	}
};

template <auto DeleteFn>
struct FunctionDeleter {
    template <class T>
    void operator()(T* ptr) {
        DeleteFn(ptr);
    }
};

template <class T, auto DeleteFn>
using unique_ptr_deleter = std::unique_ptr<T, FunctionDeleter<DeleteFn>>;

using git_annotated_commit_ptr = unique_ptr_deleter<git_annotated_commit, git_annotated_commit_free>;
using git_blob_ptr = unique_ptr_deleter<git_blob, git_blob_free>;
using git_branch_iterator_ptr = unique_ptr_deleter<git_branch_iterator, git_branch_iterator_free>;
using git_commit_ptr = unique_ptr_deleter<git_commit, git_commit_free>;
using git_diff_ptr = unique_ptr_deleter<git_diff, git_diff_free>;
using git_index_ptr = unique_ptr_deleter<git_index, git_index_free>;
using git_object_ptr = unique_ptr_deleter<git_object, git_object_free>;
using git_patch_ptr = unique_ptr_deleter<git_patch, git_patch_free>;
using git_reference_ptr = unique_ptr_deleter<git_reference, git_reference_free>;
using git_remote_ptr = unique_ptr_deleter<git_remote, git_remote_free>;
using git_repository_ptr = unique_ptr_deleter<git_repository, git_repository_free>;
using git_revwalk_ptr = unique_ptr_deleter<git_revwalk, git_revwalk_free>;
using git_signature_ptr = unique_ptr_deleter<git_signature, git_signature_free>;
using git_status_list_ptr = unique_ptr_deleter<git_status_list, git_status_list_free>;
using git_tree_ptr = unique_ptr_deleter<git_tree, git_tree_free>;

class GitAPI : public EditorVCSInterface {
	GODOT_CLASS(GitAPI, EditorVCSInterface)

	git_repository_ptr repo;

	bool has_merge = false;
	git_oid pull_merge_oid = {};
	
	// Endpoints
	bool _checkout_branch(const String branch_name);
	void _commit(const String msg);
	void _create_branch(const String branch_name);
	void _create_remote(const String remote_name, const String remote_url);
	void _discard_file(const String file_path);
	void _fetch(const String remote, const String username, const String password);
	Array _get_branch_list();
	String _get_current_branch_name();
	Array _get_diff(const String identifier, const int64_t area);
	Array _get_line_diff(const String file_path, const String text);
	Array _get_modified_files_data();
	Array _get_previous_commits(const int64_t max_commits);
	Array _get_remotes();
	String _get_vcs_name();
	bool _initialize(const String project_path);
	void _pull(const String remote, const String username, const String password);
	void _push(const String remote, const String username, const String password, const bool force);
	bool _shut_down();
	void _stage_file(const String file_path);
	void _unstage_file(const String file_path);

	// Helpers
	Array _parse_diff(git_diff *p_diff);

public:
	static void _register_methods();

	bool check_errors(int error, String message, String function, String file, int line);
	void create_gitignore_and_gitattributes();
	bool create_initial_commit();
	String get_commit_date(const git_time *intime);

	void _init();
};

} // namespace godot

#endif // !GIT_API_H
