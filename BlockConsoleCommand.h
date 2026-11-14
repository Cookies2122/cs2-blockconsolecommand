#ifndef _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_

#include <ISmmPlugin.h>
#include <filesystem.h>
#include "utlvector.h"
#include "ehandle.h"
#include <iserver.h>
#include <entity2/entitysystem.h>
#include "igameevents.h"
#include "vector.h"
#include <deque>
#include <functional>
#include <utlstring.h>
#include <keyvalues.h>
#include "CCSPlayerController.h"
#include "include/menus.h"
#include "include/utils.h"
#include "include/players.h"
#include <vector>
#include <string>
#include <algorithm>

class CServerSideClientBase;
class CNetworkGameServerBase;

class BlockConsoleCommand final : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	void AllPluginsLoaded();
	
	bool ShouldBlockCommand(const char* command);
	void LoadConfig();
	const std::vector<std::string>& GetBlockedCommands() const { return m_BlockedCommands; }
	
private:
	const char* GetAuthor();
	const char* GetName();
	const char* GetDescription();
	const char* GetURL();
	const char* GetLicense();
	const char* GetVersion();
	const char* GetDate();
	const char* GetLogTag();
	
	std::vector<std::string> m_BlockedCommands;
};

extern BlockConsoleCommand g_BlockConsoleCommand;

#endif
