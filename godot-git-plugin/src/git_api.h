#ifndef GIT_API_H
#define GIT_API_H

#include <Button.hpp>
#include <Control.hpp>
#include <EditorVCSInterface.hpp>
#include <Godot.hpp>
#include <PanelContainer.hpp>

#define memnew(m_Class) new m_Class()
#define memdelete(m_pointer) delete m_pointer

namespace godot {

class GitAPI : public EditorVCSInterface {

	GODOT_CLASS(GitAPI, EditorVCSInterface)

	godot::PanelContainer *init_settings_panel_container;
	godot::Button *init_settings_button;

public:
	static void _register_methods();

	Variant _get_commit_dock_panel_container();
	Variant _get_initialization_settings_panel_container();
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String project_root_path);
	bool _shut_down();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
