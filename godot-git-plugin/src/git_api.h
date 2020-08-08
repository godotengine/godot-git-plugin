#ifndef GIT_API_H
#define GIT_API_H

#include <Button.hpp>
#include <Control.hpp>
#include <Directory.hpp>
#include <EditorVCSInterface.hpp>
#include <File.hpp>
#include <Godot.hpp>
#include <OS.hpp>
#include <PanelContainer.hpp>

#include <allocation_defs.h>
#include <git_common.h>

#include <git2.h>

namespace godot {

class GitAPI : public EditorVCSInterface {

	GODOT_CLASS(GitAPI, EditorVCSInterface)

	const int max_commit_fetch = 10;

	bool is_initialized;
	bool can_commit;

	PanelContainer *init_settings_panel_container;
	Button *init_settings_button;

	git_repository *repo = nullptr;

	const char *remote_name = "origin";
	git_remote *remote;
	git_remote_callbacks remote_cbs;

	bool has_merge = false;
	git_oid pull_merge_oid;
	Credentials creds;

	void _commit(const String p_msg);
	bool _is_vcs_initialized();
	Dictionary _get_modified_files_data();
	Array _get_file_diff(const String file_path, int area);
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String p_project_root_path);
	bool _shut_down();
	void _stage_file(const String p_file_path);
	void _unstage_file(const String p_file_path);
	void _discard_file(String p_file_path);
	Array _get_previous_commits();
	Array _get_branch_list();
	bool _checkout_branch(String p_branch);
	Dictionary _get_data();
	void _fetch();
	void _pull();
	void _push();
	const char *_get_current_branch_name(bool full_ref);
	void _set_up_credentials(String p_username, String p_password);
	Array _parse_diff(git_diff *diff);
	Array _get_line_diff(String p_file_path, String p_text);

public:
	static void _register_methods();

	void create_gitignore_and_gitattributes();
	bool create_initial_commit();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
