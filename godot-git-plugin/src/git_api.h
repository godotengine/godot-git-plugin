#ifndef GIT_API_H
#define GIT_API_H

#include <Godot.hpp>
#include <EditorVCSInterface.hpp>

namespace godot {

class GitAPI : public EditorVCSInterface {

	GODOT_CLASS(GitAPI, EditorVCSInterface)

private:

public:
    static void _register_methods();

    GitAPI();
    ~GitAPI();

    void _init();

    void _process(float delta);
};

}

#endif // !GIT_API_H
