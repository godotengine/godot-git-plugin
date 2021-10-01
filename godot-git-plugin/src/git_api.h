#ifndef GIT_API_H
#define GIT_API_H

#include <Button.hpp>
#include <Control.hpp>
#include <Directory.hpp>
#include <EditorVCSInterface.hpp>
#include <File.hpp>
#include <Godot.hpp>
#include <PanelContainer.hpp>

#include <allocation_defs.h>
#include <git_common.h>

#include <git2.h>

namespace godot {

class GitAPI : public EditorVCSInterface {
	GODOT_CLASS(GitAPI, EditorVCSInterface)

	static GitAPI *singleton;

	bool is_initialized;
	bool can_commit;

	Array staged_files;

	PanelContainer *init_settings_panel_container;
	Button *init_settings_button;

	git_repository *repo = nullptr;

	void _commit(const String p_msg);
	bool _is_vcs_initialized();
	Dictionary _get_modified_files_data();
	Array _get_file_diff(const String file_path);
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String p_project_root_path);
	bool _shut_down();
	void _stage_file(const String p_file_path);
	void _unstage_file(const String p_file_path);

public:
	static void _register_methods();

	static GitAPI *get_singleton() { return singleton; }

	Array diff_contents;

	void create_gitignore_and_gitattributes();
	bool create_initial_commit();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
