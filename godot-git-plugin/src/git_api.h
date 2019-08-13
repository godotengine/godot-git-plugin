#ifndef GIT_API_H
#define GIT_API_H

#include <Godot.hpp>
#include <Button.hpp>
#include <Control.hpp>
#include <EditorVCSInterface.hpp>
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

	Array staged_files;
	
	PanelContainer *init_settings_panel_container;
	Button *init_settings_button;

	git_repository *repo;

	void _commit(const String p_msg);
	Control *_get_commit_dock_panel_container();
	Control *_get_initialization_settings_panel_container();
	bool _get_is_vcs_intialized();
	Dictionary _get_modified_files_data();
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String p_project_root_path);
	bool _shut_down();
	void _stage_file(const String p_file_path);
	void _unstage_file(const String p_file_path);

public:
	static void _register_methods();

	void create_gitignore();
	void create_initial_commit();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
