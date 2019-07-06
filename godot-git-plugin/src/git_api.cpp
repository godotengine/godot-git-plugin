#include "git_api.h"

#include <ClassDB.hpp>

namespace godot {

void GitAPI::_register_methods() {

    register_method("_process", &GitAPI::_process);
	register_method("get_vcs_name", &GitAPI::get_vcs_name);
}

GitAPI::GitAPI() {
}

GitAPI::~GitAPI() {
}

void GitAPI::_init() {
	godot::Godot::print("Gitinit!");
}

String GitAPI::get_vcs_name() {

	return "Git";
}

void GitAPI::_process(float delta) {
}

}
