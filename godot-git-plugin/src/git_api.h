#ifndef GIT_API_H
#define GIT_API_H

#include <Button.hpp>
#include <Control.hpp>
#include <EditorVCSInterface.hpp>
#include <Godot.hpp>
#include <PanelContainer.hpp>

#include <git2.h>

#define memnew(m_Class) new m_Class()

// NULL objects are not being handled to discourage lazy destruction of objects
#define memdelete(m_pointer) m_pointer ? WARN_PRINT("GIT API tried to delete a NULL object") : delete m_pointer;

#define GIT2_CALL(m_libgit2_function_check, m_fail_return) \
{                                                          \
	bool res = m_libgit2_function_check;                   \
	if (!res) {                                            \
		const git_error *e = giterr_last();                \
		WARN_PRINT(e->message);                            \
		return m_fail_return;                              \
	}                                                      \
}

namespace godot {

class GitAPI : public EditorVCSInterface {

	GODOT_CLASS(GitAPI, EditorVCSInterface)

	static bool is_initialized;

	PanelContainer *init_settings_panel_container;
	Button *init_settings_button;

	git_repository *repo;

	git_repository *repo;

public:
	static void _register_methods();

	Variant _get_commit_dock_panel_container();
	Variant _get_initialization_settings_panel_container();
	String _get_project_name();
	String _get_vcs_name();
	bool _initialize(const String p_project_root_path);
	bool _shut_down();

	void _init();
	void _process();

	GitAPI();
	~GitAPI();
};

} // namespace godot

#endif // !GIT_API_H
