#include "git_plugin.h"

#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/godot.hpp"

extern "C" {
int std_mbedtls_platform_init();
void std_mbedtls_platform_free();
}

void initialize_git_plugin_module(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}

	int ret = std_mbedtls_platform_init();
	if (ret) {
		ERR_PRINT("GitPlugin: Failed to initialize SSL library");
		return;
	}
	godot::ClassDB::register_class<GitPlugin>();
}

void uninitialize_git_plugin_module(godot::ModuleInitializationLevel p_level) {
	if (p_level != godot::MODULE_INITIALIZATION_LEVEL_EDITOR) {
		return;
	}
	std_mbedtls_platform_free();
}

extern "C" {

GDExtensionBool GDE_EXPORT git_plugin_init(const GDExtensionInterfaceGetProcAddress p_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_git_plugin_module);
	init_obj.register_terminator(uninitialize_git_plugin_module);
	init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_EDITOR);

	return init_obj.init();
}
}
