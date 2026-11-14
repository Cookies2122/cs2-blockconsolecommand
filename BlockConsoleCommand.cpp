#include <stdio.h>
#include <sstream>
#include <map>
#include "BlockConsoleCommand.h"
#include "metamod_oslink.h"
#include "schemasystem/schemasystem.h"
#include "utlbuffer.h"
#include "netmessages.h"
#include "serversideclient.h"
#include <keyvalues.h>
#include <cstring>
#include <cctype>
#include <khook.hpp>

BlockConsoleCommand g_BlockConsoleCommand;
PLUGIN_EXPOSE(BlockConsoleCommand, g_BlockConsoleCommand);
IVEngineServer2* engine = nullptr;
CGameEntitySystem* g_pGameEntitySystem = nullptr;
CEntitySystem* g_pEntitySystem = nullptr;
CGlobalVars *gpGlobals = nullptr;
IServerGameClients* g_pServerGameClients = nullptr;

IUtilsApi* g_pUtils;

std::map<std::string, std::string> g_vecPhrases;

static const char* kBlockedCommandsPath = "addons/configs/blockconsolecommand.txt";

KHook::Return<bool> Hook_ProcessStringCmd(CServerSideClientBase* pClient, const CNETMsg_StringCmd_t& msg);
KHook::Virtual<CServerSideClientBase, bool, const CNETMsg_StringCmd_t&> g_hkProcessStringCmd(&CServerSideClientBase::ProcessStringCmd, &Hook_ProcessStringCmd, nullptr);

void HookClient(int slot);
void UnhookClient(int slot);

std::string ToLower(const std::string& str)
{
	std::string result = str;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

std::string GetFirstWord(const std::string& str)
{
	size_t start = str.find_first_not_of(" \t");
	if (start == std::string::npos)
		return "";
	
	size_t end = str.find_first_of(" \t", start);
	if (end == std::string::npos)
		return str.substr(start);
	
	return str.substr(start, end - start);
}

bool BlockConsoleCommand::ShouldBlockCommand(const char* command)
{
	if (!command || !*command)
		return false;
	
	std::string cmdStr = ToLower(command);
	std::string firstWord = ToLower(GetFirstWord(command));
	
	for (const auto& blocked : m_BlockedCommands)
	{
		std::string blockedLower = ToLower(blocked);

		if (firstWord == blockedLower)
			return true;

		if (cmdStr.find(blockedLower) == 0)
		{
			if (cmdStr.length() == blockedLower.length() || 
				cmdStr[blockedLower.length()] == ' ' || 
				cmdStr[blockedLower.length()] == '\t')
			{
				return true;
			}
		}
	}
	
	return false;
}

static std::string Trim(const std::string& text)
{
	const auto first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) return "";
	const auto last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

void BlockConsoleCommand::LoadConfig()
{
	m_BlockedCommands.clear();
	
	if (!g_pFullFileSystem)
	{
		return;
	}
	
	CUtlBuffer buf;
	if (!g_pFullFileSystem->ReadFile(kBlockedCommandsPath, "GAME", buf))
	{
		ConColorMsg(Color(255, 165, 0, 255), "[BlockConsoleCommand] Config not found at %s\n", kBlockedCommandsPath);
		return;
	}
	
	std::string content;
	content.assign(static_cast<const char*>(buf.Base()), buf.TellPut());
	
	std::istringstream iss(content);
	std::string line;
	while (std::getline(iss, line))
	{
		line = Trim(line);
		
		if (line.size() >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF)
			line = line.substr(3);
		if (line.empty())
			continue;
		if (line.rfind("#", 0) == 0 || line.rfind("//", 0) == 0)
			continue;
		
		m_BlockedCommands.push_back(line);
	}
	
	ConColorMsg(Color(0, 255, 0, 255), "[BlockConsoleCommand] Loaded %zu blocked commands from config\n", 
		m_BlockedCommands.size());
}

void LoadTranslations()
{
	KeyValues::AutoDelete g_kvPhrases("Phrases");
	const char *pszPath = "addons/translations/blockconsolecommand.phrases.txt";
	if (!g_kvPhrases->LoadFromFile(g_pFullFileSystem, pszPath))
	{
		ConColorMsg(Color(255, 165, 0, 255), "[BlockConsoleCommand] Failed to load %s\n", pszPath);
		return;
	}

	std::string szLanguage = std::string(g_pUtils->GetLanguage());
	const char* g_pszLanguage = szLanguage.c_str();
	for (KeyValues *pKey = g_kvPhrases->GetFirstTrueSubKey(); pKey; pKey = pKey->GetNextTrueSubKey())
		g_vecPhrases[std::string(pKey->GetName())] = std::string(pKey->GetString(g_pszLanguage));
	
	ConColorMsg(Color(0, 255, 0, 255), "[BlockConsoleCommand] Loaded %zu phrases\n", g_vecPhrases.size());
}

KHook::Return<bool> Hook_ProcessStringCmd(CServerSideClientBase* pClient, const CNETMsg_StringCmd_t& msg)
{
	const char* command = msg.command().c_str();

	if (g_BlockConsoleCommand.ShouldBlockCommand(command))
	{
		if (pClient)
		{
			g_pUtils->PrintToConsole(pClient->GetPlayerSlot().Get(), g_vecPhrases["CommandBlocked"].c_str());
		}
		return { KHook::Action::Supersede, true };
	}

	return { KHook::Action::Ignore, true };
}

static bool g_ClientHooked[64] = {false};

void HookClient(int slot)
{
	if (slot < 0 || slot >= 64 || g_ClientHooked[slot])
		return;
	
	if (!g_pNetworkServerService)
		return;
	
	CNetworkGameServerBase* pServer = g_pNetworkServerService->GetIGameServer();
	if (!pServer)
		return;
	
	CServerSideClientBase* pClient = pServer->GetClientBySlot(CPlayerSlot(slot));
	if (!pClient)
		return;
	
	g_hkProcessStringCmd.Add(pClient);
	g_ClientHooked[slot] = true;
}

void UnhookClient(int slot)
{
	if (slot < 0 || slot >= 64 || !g_ClientHooked[slot])
		return;
	
	if (!g_pNetworkServerService)
		return;
	
	CNetworkGameServerBase* pServer = g_pNetworkServerService->GetIGameServer();
	if (!pServer)
		return;
	
	CServerSideClientBase* pClient = pServer->GetClientBySlot(CPlayerSlot(slot));
	if (!pClient)
		return;
	
	g_hkProcessStringCmd.Remove(pClient);
	g_ClientHooked[slot] = false;
}

void HookAllClients()
{
	if (!g_pNetworkServerService)
		return;
	
	CNetworkGameServerBase* pServer = g_pNetworkServerService->GetIGameServer();
	if (!pServer)
		return;
	
	for (int i = 0; i < 64; i++)
	{
		CServerSideClientBase* pClient = pServer->GetClientBySlot(CPlayerSlot(i));
		if (pClient && pClient->IsConnected())
		{
			HookClient(i);
		}
	}
}

void UnhookAllClients()
{
	for (int i = 0; i < 64; i++)
	{
		UnhookClient(i);
	}
}

void StartupServer()
{
	g_pGameEntitySystem = GameEntitySystem();
	g_pEntitySystem = g_pUtils->GetCEntitySystem();
	gpGlobals = g_pUtils->GetCGlobalVars();
	
	HookAllClients();
	
	g_pUtils->HookEvent(g_PLID, "player_connect_full", [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
	{
		int slot = pEvent->GetInt("userid");
		HookClient(slot);
	});
	
	g_pUtils->HookEvent(g_PLID, "player_disconnect", [](const char* szName, IGameEvent* pEvent, bool bDontBroadcast)
	{
		int slot = pEvent->GetInt("userid");
		UnhookClient(slot);
	});
}

bool BlockConsoleCommand::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, g_pCVar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetEngineFactory, g_pSchemaSystem, ISchemaSystem, SCHEMASYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer2, SOURCE2ENGINETOSERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pServerGameClients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetFileSystemFactory, g_pFullFileSystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

	g_SMAPI->AddListener( this, this );
	
	LoadConfig();

	if (late)
	{
		HookAllClients();
	}

	return true;
}

bool BlockConsoleCommand::Unload(char *error, size_t maxlen)
{
	int ret;
	if (!g_SMAPI->MetaFactory(UTILS_INTERFACE, &ret, nullptr)) g_pUtils = nullptr;
	UnhookAllClients();
	if (g_pUtils) g_pUtils->ClearAllHooks(g_PLID);
	
	ConVar_Unregister();
	
	return true;
}

void BlockConsoleCommand::AllPluginsLoaded()
{
	char error[64];
	int ret;
	g_pUtils = (IUtilsApi *)g_SMAPI->MetaFactory(UTILS_INTERFACE, &ret, NULL);
	if (ret == META_IFACE_FAILED)
	{
		g_SMAPI->Format(error, sizeof(error), "Missing Utils system plugin");
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		std::string sBuffer = "meta unload "+std::to_string(g_PLID);
		engine->ServerCommand(sBuffer.c_str());
		return;
	}
	LoadTranslations();
	g_pUtils->StartupServer(g_PLID, StartupServer);
}

const char* BlockConsoleCommand::GetLicense()
{
	return "GPL";
}

const char* BlockConsoleCommand::GetVersion()
{
	return "1.0";
}

const char* BlockConsoleCommand::GetDate()
{
	return __DATE__;
}

const char *BlockConsoleCommand::GetLogTag()
{
	return "[BlockConsoleCommand]";
}

const char* BlockConsoleCommand::GetAuthor()
{
	return "ABKAM";
}

const char* BlockConsoleCommand::GetDescription()
{
	return "Block Console Command";
}

const char* BlockConsoleCommand::GetName()
{
	return "Block Console Command";
}

const char* BlockConsoleCommand::GetURL()
{
	return "https://discord.gg/ChYfTtrtmS";
}
