#ifndef GIT_API_H
#define GIT_API_H

#include <Button.hpp>
#include <Control.hpp>
#include <EditorVCSInterface.hpp>
#include <Godot.hpp>
#include <PanelContainer.hpp>
#include <Directory.hpp>
#include <File.hpp>

#include <git2.h>

#include <git_common.h>
#include <allocation_defs.h>
#include <git_callbacks.h>

namespace godot {

class GitAPI : public EditorVCSInterface {

	GODOT_CLASS(GitAPI, EditorVCSInterface)

	static bool is_initialized;
	static bool is_repo_open;

	PanelContainer *init_settings_panel_container;
	Button *init_settings_button;

	git_repository *repo;
	git_signature *author;
	git_signature *committer;

public:
	static void _register_methods();

	void _commit(const String msg);
	bool _get_is_vcs_intialized();
	String _get_project_name();
	Object *_get_commit_dock_panel_container();
	Object *_get_initialization_settings_panel_container();
	Dictionary _get_untracked_files_data();
	String _get_vcs_name();
	bool _initialize(const String project_root_path);
	bool _shut_down();
	void _stage_all();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
