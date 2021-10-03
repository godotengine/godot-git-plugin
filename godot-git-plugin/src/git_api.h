#ifndef GIT_API_H
#define GIT_API_H

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

class GitAPI : public EditorVCSInterface {
	GODOT_CLASS(GitAPI, EditorVCSInterface)

	const int max_commit_fetch = 10;

	bool is_initialized;
	bool can_commit;

	git_repository *repo = nullptr;

	bool has_merge = false;
	git_oid pull_merge_oid;
	
	// Endpoints
	bool _checkout_branch(const String branch);
	void _commit(const String message);
	void _discard_file(const String file_path);
	void _fetch(const String remote, const String username, const String password);
	Array _get_branch_list();
	String _get_current_branch_name(const bool full_ref);
	Array _get_file_diff(const String identifier, const int64_t area);
	Array _get_line_diff(const String file_path, const String text);
	Array _get_modified_files_data();
	Array _get_previous_commits();
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String project_root_path);
	bool _is_vcs_initialized();
	void _pull(const String remote, const String username, const String password);
	void _push(const String remote, const String username, const String password);
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

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
