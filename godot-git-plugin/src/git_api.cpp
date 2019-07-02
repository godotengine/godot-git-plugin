#include "git_api.h"

namespace godot {

void GitAPI::_register_methods() {

    register_method("_process", &GitAPI::_process);
}

GitAPI::GitAPI() {

	register_singleton((Node *)this);
}

GitAPI::~GitAPI() {
}

void GitAPI::_init() {

	WARN_PRINT("Gitinit!");
}

void GitAPI::_process(float delta) {
}

}
