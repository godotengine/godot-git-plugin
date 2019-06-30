#include "git_api.h"

namespace godot {

void GitAPI::_register_methods() {

    register_method("_process", &GitAPI::_process);
}

GitAPI::GitAPI() {
}

GitAPI::~GitAPI() {
}

void GitAPI::_init() {

	EditorVCS::replace_singleton((Node *) this);
}

void GitAPI::_process(float delta) {
}

}
