#ifndef GIT_API_H
#define GIT_API_H

#include <Godot.hpp>
#include <Sprite.hpp>

namespace godot {

    class GitAPI : public Sprite {

        GODOT_CLASS(GitAPI, Sprite) // This base class is temporary

    private:
        float time_passed;

    public:
        static void _register_methods();

        GitAPI();
        ~GitAPI();

        void _init();

        void _process(float delta);
    };
}

#endif // !GIT_API_H
