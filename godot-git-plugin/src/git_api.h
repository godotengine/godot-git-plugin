#ifndef GIT_API_H
#define GIT_API_H

#include <Godot.hpp>
#include <EditorVCS.hpp>

namespace godot {

    class GitAPI : public EditorVCS {

        GODOT_CLASS(GitAPI, EditorVCS) // This base class is temporary

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
