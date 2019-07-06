#include "git_api.h"
#include "vcs_interface_struct.h"

#include <Godot.hpp>

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {

	godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_singleton(godot_gdnative_init_options *o) {

	godot::WARN_PRINT("godot_gdnative_singleton");

	godot::VCSInterface *new_vcs_api = new godot::VCSInterface();
	new_vcs_api->get_vcs_name = godot::GitAPI::_get_vcs_name;

	godot::EditorVCSInterface::set_vcs_api_struct(new_vcs_api);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {

    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {

    godot::Godot::nativescript_init(handle);
}
