#pragma once
#include "cbase.h"

/**
 * Interface for Crispy Fortress game plugins
 * to adhere to.
 */
struct ICrispyPlugin {

	// default virtual destructor to enable RTTI
	virtual ~ICrispyPlugin() = default;

	virtual void Init() {};

	virtual void Tick() {};

	virtual void Shutdown() {};

};

/**
 * Register a plugin with the Crispy Fortress game system.
 * 
 * \param[in] plugin Pointer to the plugin to register.
 * \param[in] name Name of the plugin.
 * 
 * \returns True on success, false otherwise
 */
bool CF_Register(ICrispyPlugin* plugin, const char* name);

/**
 * Register helper thing.
 * 
 * \tparam Plugin Plugin template type.
 */
template<class Plugin>
struct PluginRegisterHelper {
	static_assert(std::is_base_of_v<ICrispyPlugin, Plugin>, "Plugin must inherit from ICrispyPlugin, you fuck");

	PluginRegisterHelper(Plugin& plugin, const char* name) {
		CF_Register((ICrispyPlugin*)&plugin, name);
	}

};

// macro to link a plugin with the crispy fortress game system.
#define CF_LINK_PLUGIN(T, name) \
	static T _plugin_##T; \
	static PluginRegisterHelper<T> __plugin_register_##T(_plugin_##T, name);