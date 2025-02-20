#include "cbase.h"
#include "crispyfortress.h"


struct CCrispyFortress : public CAutoGameSystemPerFrame {

	CCrispyFortress() 
		: CAutoGameSystemPerFrame("Crispy Fortress") {

	}

	ICrispyPlugin* GetPluginByName(const char* name) {
		for(auto& plugin : plugins) {
			if (FStrEq(plugin.name, name))
				return plugin.plugin;
		}

		// lookup failed
		return nullptr;
	}

	void FrameUpdatePreEntityThink() {
		for(auto& plugin : plugins) {
			plugin.plugin->Tick();
		}
	}

	bool RegisterPlugin(ICrispyPlugin* plugin, const char* name) { 
		if (!plugin)
			return false;

		RegisteredPlugin registered;

		// Do not register duplicate plugins
		if (GetPluginByName(name) != nullptr)
			return false;

		DevMsg("%s says we registered %s\n", __FUNCTION__, name);

		// Welp now we can bother to set it up then
		registered.name = name;
		registered.plugin = plugin;

		plugin->Init();
		 
		plugins.AddToTail(registered);

		return true;
	}

	void Shutdown() {
		for(auto& plugin : plugins) {
			plugin.plugin->Shutdown();
		}
	}

private:

	struct RegisteredPlugin {
		ICrispyPlugin* plugin;
		const char* name;
	};

	CUtlVector<RegisteredPlugin> plugins;
};

/**
 * The global instance of the Crispy Fortress game system.
 */
static CCrispyFortress g_Crispy;

bool CF_Register(ICrispyPlugin* plugin, const char* name) {
	return g_Crispy.RegisterPlugin(plugin, name);
}

struct MyCFPlugin : public ICrispyPlugin {

	int ticktestcl = 0;
	int ticktestsrv = 0;

	void Init() {
		Msg("MyCFPlugin::Init() says hello\n");
	}

	void Tick() {
#ifdef CLIENT_DLL
		ticktestcl++;
#else
		ticktestsrv++;
#endif
	}

	void Shutdown() {
	
	}

#ifdef CLIENT_DLL
	CON_COMMAND_MEMBER_F( MyCFPlugin, "cf_ticktest_cl", CC_CFTickTest_f, "the onceler died", FCVAR_NONE );
#else
	CON_COMMAND_MEMBER_F( MyCFPlugin, "cf_ticktest_srv", CC_CFTickTest_f, "oh no", FCVAR_NONE );
#endif

};

// nasty, but Valve macros do *not* allow inplace usage
void MyCFPlugin::CC_CFTickTest_f(const CCommand &args) {
	Msg("this is ticky ~ client: %d, server: %d\n", ticktestcl, ticktestsrv);
}

CF_LINK_PLUGIN(MyCFPlugin, "My Crispy Plugin");