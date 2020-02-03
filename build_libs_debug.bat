set GODOT_PATH_RELATIVE_TO_PLUGIN="..\godot\bin"
set GIT_PLUGIN_RELATIVE_TO_GODOT="..\..\godot-git-plugin\"
git submodule init
git submodule update --init --recursive
cd %GODOT_PATH_RELATIVE_TO_PLUGIN%
godot.windows.tools.64.exe --gdnative-generate-json-api api.json
copy api.json %GIT_PLUGIN_RELATIVE_TO_GODOT%\api.json /Y
cd %GIT_PLUGIN_RELATIVE_TO_GODOT%\godot-cpp\
scons platform=windows target=debug generate_bindings=yes use_custom_api_file=yes custom_api_file=../api.json bits=64
