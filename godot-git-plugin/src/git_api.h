#ifndef GIT_API_H
#define GIT_API_H

#include <Godot.hpp>
#include <Control.hpp>
#include <PanelContainer.hpp>
#include <Button.hpp>
#include <EditorVCSInterface.hpp>

namespace godot {

class GitAPI : public EditorVCSInterface {

	GODOT_CLASS(GitAPI, EditorVCSInterface)

	godot::PanelContainer *panel_container;
	godot::Button *button;

public:
	static void _register_methods();

	bool initialize(const String project_root_path);
	Control *get_commit_dock_panel_container();
	Control *get_initialization_settings_panel_container();
	bool shut_down();

	String get_project_name();
	String get_vcs_name();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

}

#endif // !GIT_API_H
