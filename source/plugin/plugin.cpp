/****************************************************************************
 * Copyright (C) 2012 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <algorithm>
#include "plugin.hpp"
#include "fileOps/fileOps.h"
#include "gui/text.hpp"
#include "gecko/gecko.hpp"
#include "devicemounter/PartitionHandle.h"
#include "devicemounter/DeviceHandler.hpp"
#include "types.h"
#include "crc32.h"

// For PS1 serial
#ifdef MSB_FIRST
#define MODETEST_VAL    0x00ffffff
#else
#define MODETEST_VAL    0xffffff00
#endif

Plugin m_plugin;
void Plugin::init(const string& m_pluginsDir)
{
	PluginMagicWord[8] = '\0';
	pluginsDir = m_pluginsDir;
	Plugins.clear();
	adding = true;
}

static bool PluginOptions_cmp(PluginOptions lhs, PluginOptions rhs)
{
	const wchar_t *first = lhs.DisplayName.c_str();
	const wchar_t *second = rhs.DisplayName.c_str();
	return wchar_cmp(first, second, wcslen(first), wcslen(second));
}

void Plugin::EndAdd()
{
	std::sort(Plugins.begin(), Plugins.end(), PluginOptions_cmp);
	adding = false;
}

void Plugin::Cleanup()
{
	Plugins.clear();
	adding = true;
}

bool Plugin::AddPlugin(Config &plugin)
{
	if(!adding)
		return false;

	PluginOptions NewPlugin;
	NewPlugin.DolName = plugin.getString(PLUGIN, "dolFile");
	NewPlugin.coverFolder = plugin.getString(PLUGIN, "coverFolder");
	NewPlugin.magic = strtoul(plugin.getString(PLUGIN, "magic").c_str(), NULL, 16);
	NewPlugin.caseColor = strtoul(plugin.getString(PLUGIN, "coverColor").c_str(), NULL, 16);
	NewPlugin.romPartition = plugin.getInt(PLUGIN, "rompartition", -1);
	NewPlugin.romDir = plugin.getString(PLUGIN, "romDir");
	NewPlugin.fileTypes = plugin.getString(PLUGIN, "fileTypes");
	NewPlugin.Args = plugin.getStrings(PLUGIN, "arguments", '|');
	NewPlugin.boxMode = plugin.getBool(PLUGIN, "boxmode", 1);
	string PluginName = plugin.getString(PLUGIN, "displayname");
	if(PluginName.size() < 2)
	{
		PluginName = NewPlugin.DolName;
		PluginName.erase(PluginName.end() - 4, PluginName.end());
	}
	NewPlugin.DisplayName.fromUTF8(PluginName.c_str());
	NewPlugin.consoleCoverID = plugin.getString(PLUGIN,"consoleCoverID");

	const string &bannerfilepath = sfmt("%s/%s", pluginsDir.c_str(), plugin.getString(PLUGIN,"bannerSound").c_str());
	fsop_GetFileSizeBytes(bannerfilepath.c_str(), &NewPlugin.BannerSoundSize);
	if(NewPlugin.BannerSoundSize > 0)
		NewPlugin.BannerSound = bannerfilepath;
	Plugins.push_back(NewPlugin);
	return false;
}

s8 Plugin::GetPluginPosition(u32 magic)
{
	for(u8 pos = 0; pos < Plugins.size(); pos++)
	{
		if(magic == Plugins[pos].magic)
			return pos;
	}
	return -1;
}

u32 Plugin::getPluginMagic(u8 pos)
{
	return Plugins[pos].magic;
}

u8* Plugin::GetBannerSound(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
	{
		u32 size = 0;
		return fsop_ReadFile(Plugins[Plugin_Pos].BannerSound.c_str(), &size);
	}
	return NULL;
}

u32 Plugin::GetBannerSoundSize()
{
	//We call that directly after GetBannerSound, so no need to search for the magic again
	if(Plugin_Pos >= 0)
		return Plugins[Plugin_Pos].BannerSoundSize;
	return 0;
}

const char *Plugin::GetDolName(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return Plugins[Plugin_Pos].DolName.c_str();
	return NULL;
}

const char *Plugin::GetCoverFolderName(u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
		return Plugins[Plugin_Pos].coverFolder.c_str();
	return NULL;
}

int Plugin::GetRomPartition(u8 pos)
{
	return Plugins[pos].romPartition;
}

const char *Plugin::GetRomDir(u8 pos)
{
	return Plugins[pos].romDir.c_str();
}

const string& Plugin::GetFileTypes(u8 pos)
{
	return Plugins[pos].fileTypes;
}

u32 Plugin::GetCaseColor(u8 pos)
{
	return Plugins[pos].caseColor;
}

bool Plugin::GetBoxMode(u8 pos)
{
	return Plugins[pos].boxMode;
}

wstringEx Plugin::GetPluginName(u8 pos)
{
	return Plugins[pos].DisplayName;
}

bool Plugin::PluginExist(u8 pos)
{
	if(pos < Plugins.size())
		return true;
	return false;
}

void Plugin::SetEnablePlugin(Config &cfg, u8 pos, u8 ForceMode)
{
	if(pos < Plugins.size())
	{
		strncpy(PluginMagicWord, fmt("%08x", Plugins[pos].magic), 8);
		if(ForceMode == 1)
			cfg.setBool(PLUGIN_ENABLED, PluginMagicWord, false);
		else if(ForceMode == 2)
			cfg.setBool(PLUGIN_ENABLED, PluginMagicWord, true);
		else
			cfg.setBool(PLUGIN_ENABLED, PluginMagicWord, cfg.getBool(PLUGIN_ENABLED, PluginMagicWord) ? false : true);
	}
}

bool Plugin::GetEnableStatus(Config &cfg, u32 magic)
{
	if((Plugin_Pos = GetPluginPosition(magic)) >= 0)
	{
		strncpy(PluginMagicWord, fmt("%08x", magic), 8);
		return cfg.getBool(PLUGIN_ENABLED, PluginMagicWord, true);
	}
	return false;
}

const vector<bool> &Plugin::GetEnabledPlugins(Config &cfg, u8 *num)
{
	enabledPlugins.clear();
	u8 enabledPluginsNumber = 0;
	for(u8 i = 0; i < Plugins.size(); i++)
	{
		strncpy(PluginMagicWord, fmt("%08x", Plugins[i].magic), 8);
		if(cfg.getBool(PLUGIN_ENABLED, PluginMagicWord, true))
		{
			enabledPluginsNumber++;
			enabledPlugins.push_back(true);
		}
		else
			enabledPlugins.push_back(false);
	}
	if(enabledPluginsNumber == Plugins.size())
		enabledPlugins.clear();
	if(num != NULL)
		*num = enabledPluginsNumber;
	return enabledPlugins;
}

vector<dir_discHdr> Plugin::ParseScummvmINI(Config &ini, const char *Device, u32 Magic)
{
	gprintf("Parsing scummvm.ini\n");
	vector<dir_discHdr> gameHeader;
	if(!ini.loaded())
		return gameHeader;

	const char *GameDomain = ini.firstDomain().c_str();
	dir_discHdr ListElement;
	while(1)
	{
		if(strlen(GameDomain) < 2)
			break;
		char GameName[64];
		memset(GameName, 0, sizeof(GameName));
		strncpy(GameName, ini.getString(GameDomain, "description").c_str(), 63);
		if(strlen(GameName) < 2 || strncasecmp(Device, ini.getString(GameDomain, "path").c_str(), 2) != 0)
		{
			GameDomain = ini.nextDomain().c_str();
			continue;
		}
		memset((void*)&ListElement, 0, sizeof(dir_discHdr));
		memcpy(ListElement.id, PLUGIN, 6);
		ListElement.casecolor = Plugins.back().caseColor;
		mbstowcs(ListElement.title, GameName, 63);
		strncpy(ListElement.path, GameDomain, sizeof(ListElement.path));
		gprintf("Found: %s\n", GameDomain);
		ListElement.settings[0] = Magic;
		ListElement.type = TYPE_PLUGIN;
		gameHeader.push_back(ListElement);
		GameDomain = ini.nextDomain().c_str();
	}
	return gameHeader;
}

vector<string> Plugin::CreateArgs(const char *device, const char *path,
			const char *title, const char *loader, u32 title_len_no_ext, u32 magic)
{
	vector<string> args;
	Plugin_Pos = GetPluginPosition(magic);
	if(Plugin_Pos < 0)
		return args;
	for(vector<string>::const_iterator arg = Plugins[Plugin_Pos].Args.begin();
								arg != Plugins[Plugin_Pos].Args.end(); ++arg)
	{
		string Argument(*arg);
		if(Argument.find(PLUGIN_DEV) != string::npos)
			Argument.replace(Argument.find(PLUGIN_DEV), strlen(PLUGIN_DEV), device);
		if(Argument.find(PLUGIN_PATH) != string::npos)
			Argument.replace(Argument.find(PLUGIN_PATH), strlen(PLUGIN_PATH), path);
		if(Argument.find(PLUGIN_NAME) != string::npos)
			Argument.replace(Argument.find(PLUGIN_NAME), strlen(PLUGIN_NAME), title);
		if(Argument.find(PLUGIN_LDR) != string::npos)
			Argument.replace(Argument.find(PLUGIN_LDR), strlen(PLUGIN_LDR), loader);
		if(Argument.find(PLUGIN_NOEXT) != string::npos)
			Argument.replace(Argument.find(PLUGIN_NOEXT), strlen(PLUGIN_NOEXT), title, title_len_no_ext);
		args.push_back(Argument);
	}
	return args;
}

/* Give the current game a simplified name */
string Plugin::GetRomName(const dir_discHdr *gameHeader)
{
	if(strrchr(gameHeader->path, '/') != NULL)
	{
		// Remove extension
		string FullName = strrchr(gameHeader->path, '/') + 1;
		FullName = FullName.substr(0, FullName.find_last_of("."));

		// Remove common suffixes and replace unwanted characters. 
		string ShortName = FullName.substr(0, FullName.find(" (")).substr(0, FullName.find(" ["));
		replace(ShortName.begin(), ShortName.end(), '_', ' ');
		return ShortName;
	}
	return NULL;
}

/* Get serial from PS1 header's iso (Borrowed from Retroarch with a few c++ changes)*/
static int GetSerialPS1(const char *path, string &Serial, int sub_channel_mixed)
{
	char *tmp;
	int skip, frame_size, is_mode1, cd_sector;
	char buffer[2048 * 2];

	ifstream fp;
	fp.open(path, ios::binary);

	buffer[0] = '\0';
	is_mode1  = 0;

	if ( !fp.seekg(0, ios::end) )
		goto error;

	if (!sub_channel_mixed)
	{
		if ( (!fp.tellg()) & 0x7FF)
		{
			unsigned int mode_test = 0;

			if ( !fp.seekg(0, ios::beg) )
				goto error;

			fp.read(reinterpret_cast<char *>(mode_test), 4);

			if (mode_test != MODETEST_VAL)
				is_mode1 = 1;
		}
	}

	skip = is_mode1? 0: 24;
	frame_size = sub_channel_mixed? 2448: is_mode1? 2048: 2352;

	if ( !fp.seekg(156 + skip + 16 * frame_size, ios::beg) )
		goto error;

	fp.read(buffer, 6);

	cd_sector = buffer[2] | (buffer[3] << 8) | (buffer[4] << 16);

	if ( !fp.seekg(skip + cd_sector * frame_size, ios::beg) )
	goto error;

	fp.read(buffer, 2048 * 2);

	tmp = buffer;
	while (tmp < (buffer + 2048 * 2))
	{
		if (!*tmp)
		goto error;

		if (!strncasecmp((const char*)(tmp + 33), "SYSTEM.CNF;1", 12))
			break;
		tmp += *tmp;
	}

	if(tmp >= (buffer + 2048 * 2))
		goto error;

	cd_sector = tmp[2] | (tmp[3] << 8) | (tmp[4] << 16);
	if ( !fp.seekg(skip + cd_sector * frame_size, ios::beg) )
		goto error;

	fp.read(buffer, 256);
	buffer[256] = '\0';

	tmp = buffer;
	while(*tmp && strncasecmp((const char*)tmp, "boot", 4))
		tmp++;

	if(!*tmp)
		goto error;

	Serial = tmp;
	Serial.erase(0, Serial.find_first_of('\\') + 1); 
	replace(Serial.begin(), Serial.end(), '_', '-');
	Serial.erase(remove(Serial.begin(), Serial.end(), '.'), Serial.end());
	Serial.erase(Serial.find_first_of(';'));

	fp.close();

	return 1;

error:
	return 0;
}

/* Get serial from MegaCD header's iso 
*
* All headers should have "SEGADISCSYSTEM" in the first bytes.
* The offset differs, the serial search depends on it.    
*/

void GetSerialMegaCD(const char *path, string &Serial)
{
	ifstream infile;
	char buf[10];

	infile.open(path, ios::binary);
	infile.seekg(0, ios::beg);
	infile.read ((char*)buf, 4);
	buf[4] = '\0';

	// iso or bin offset.
	if (!strcasecmp(buf, "SEGA"))
		infile.seekg(0x182, ios::beg);
	else
	{
		infile.seekg(0x10, ios::beg);
		infile.read ((char*)buf, 4);

		if (!strcasecmp(buf, "SEGA"))
			infile.seekg(0x192, ios::beg);
	}

	infile.read(buf, 9);
	buf[9] = '\0';
	infile.close();

	Serial = buf;
	Serial.erase(remove(Serial.begin(), Serial.end(), ' '), Serial.end());

	// Cut at second dash, we don't need any extra code. 
	// Sonic CD : MK-4407(-00)
	size_t dash = std::count(Serial.begin(), Serial.end(), '-');

	if(dash > 1)
		Serial.erase(Serial.find_last_of('-'));

	infile.close();
}


/* Get the Game ID based on name or CRC/Serial
*
* It returns the ID used to search in the platform database(SUPERNES.xml for instance)
* and can also be used for snapshots/cartriges/discs images.
* The Game ID is a 6 length alphanumerical value. It's screenscraper ID filled with 'A' letter.
*/
string Plugin::GetRomId(const dir_discHdr *gameHeader, const char *datadir, char *platform, const string &name)
{
	string GameID;
	string CRC_Serial(12, '*');

	// Load a platform list that is used to identify the current game.
	// It contains a default filename(preferably No-intro without region flag), the GameID and then all known CRC32/serials.
	// filename=GameID|crc1|crc2|etc...
	// For example in SUPERNES.ini : Super Aleste=2241AA|5CA5781B|...
	Config m_crc;
	m_crc.unload();
	m_crc.load(fmt("%s/%s/%s.ini", datadir, platform, platform) );

	bool found_name = false;
	found_name = m_crc.has(platform, name.c_str());

	// Get game ID based on the filename
	if(found_name)
	{
		vector<string> searchID = m_crc.getStrings(platform, name.c_str(), '|');

		if(!searchID.empty())
		{
			GameID = searchID[0];
		}

		m_crc.unload();
	}
	// Get game ID by CRC or serial
	else
	{
		m_crc.unload();

		char crc_string[9];
		crc_string[0] = '\0';
		u32 buffer;
		ifstream infile;

		// For arcade games use the crc zip
		if(strcasestr(platform, "ARCADE") || strcasestr(platform, "CPS") || !strncasecmp(platform, "NEOGEO", 6))
		{
			strncpy(crc_string, fmt("%08x", crc32file(gameHeader->path)), 8);
			crc_string[8] = '\0';
		}
		else
		{
			// Look for for the file's crc inside the archive 
			if(strstr(gameHeader->path, ".zip") != NULL)
			{
				infile.open(gameHeader->path, ios::binary);
				infile.seekg(0x0e, ios::beg);
				infile.read((char*)&buffer, 8);
				infile.close();
				strncpy(crc_string, fmt("%08x", (u32)__builtin_bswap32(buffer)), 8);
				crc_string[8] = '\0';
			}
			else
			{
				// Check a serial in header's file instead of crc for these CD based platforms.
				// CRC calculation would take up to 30 seconds!
				if(!strcasecmp(platform, "MEGACD"))
				{
					GetSerialMegaCD(gameHeader->path, CRC_Serial);
				}
				else if(!strcasecmp(platform, "PS1"))
				{
					bool found;
					found = GetSerialPS1(gameHeader->path, CRC_Serial, 0);

					if(!found)
					GetSerialPS1(gameHeader->path, CRC_Serial, 1);
				}
				else if(!strcasecmp(platform, "ATARIST"))
				{
					s8 pos = m_plugin.GetPluginPosition(gameHeader->settings[0]);
					string FileTypes = m_plugin.GetFileTypes(pos);
					string path;

					// Parse config to get floppy A path
					if(strcasestr(FileTypes.c_str(), ".cfg"))
					{
						Config m_cfg;
						m_cfg.unload();
						m_cfg.load(fmt("%s", gameHeader->path) );
						path = m_cfg.getString("Floppy", "szDiskAFileName", "");
						m_cfg.unload();

						// Replace usb:/ with usb1:/ if needed
						if (path.find("usb:/") != std::string::npos)
						{
							path.insert(3, "1");
						}
					}
					else
					{
						path = gameHeader->path;
					}

					if (path.find(".zip") != std::string::npos)
					{
						infile.open(path, ios::binary);
						infile.seekg(0x0e, ios::beg);
						infile.read((char*)&buffer, 8);
						infile.close();
						strncpy(crc_string, fmt("%08x", (u32)__builtin_bswap32(buffer)), 8);
						crc_string[8] = '\0';
					}
					else
					{
						strncpy(crc_string, fmt("%08x", crc32file(path.c_str())), 8);
						crc_string[8] = '\0';
					}
				}
				// Just check CRC for a regular file on any other system.
				else
				{
					strncpy(crc_string, fmt("%08x", crc32file(gameHeader->path)), 8);
					crc_string[8] = '\0';
				}
			}
		}
		
		if(crc_string[0] != '\0')
			CRC_Serial = crc_string;

		// Now search ID with the obtained CRC/Serial
		// Just add 2 pipes in the pattern to be sure we don't find a crc instead of ID
		size_t idx;
		idx=CRC_Serial.length();
		CRC_Serial.insert(0, "|").insert(idx+1, "|");

		ifstream inputFile;
		inputFile.open( fmt("%s/%s/%s.ini", datadir, platform, platform) );
		string line;

		while(getline(inputFile, line))
		{
			// FIXME ahem, ignore case... 
			if (line.find(lowerCase( CRC_Serial ), 0) != string::npos || line.find(upperCase( CRC_Serial ), 0) != string::npos)
			{
				unsigned first = (line.find('=') + 1);
				unsigned last = line.find_first_of('|');
				string ID = line.substr (first,last-first);

				if(ID.empty())
				{
					return "";
				}

				GameID = ID;
				break;
			}
		}
	}
	
	return GameID;
}

string Plugin::GenerateCoverLink(dir_discHdr gameHeader, const string& constURL, Config &Checksums)
{
	string url(constURL);
	Plugin_Pos = GetPluginPosition(gameHeader.settings[0]);

	if(url.find(TAG_LOC) != url.npos)
 		url.replace(url.find(TAG_LOC), strlen(TAG_LOC), "EN");

	if(url.find(TAG_CONSOLE) != url.npos)
		url.replace(url.find(TAG_CONSOLE), strlen(TAG_CONSOLE), (Plugins[Plugin_Pos].consoleCoverID.size() ? Plugins[Plugin_Pos].consoleCoverID.c_str() : "nintendo"));	

	char gamePath[256];
	if(string(gameHeader.path).find_last_of("/") != string::npos)
		strncpy(gamePath, &gameHeader.path[string(gameHeader.path).find_last_of("/")+1], sizeof(gamePath));
	else
		strncpy(gamePath, gameHeader.path, sizeof(gamePath));
	const string& cachedCRC = Checksums.getString("CHECKSUMS", gamePath);
	char crc_string[9];
	crc_string[8] = '\0';
	if(cachedCRC.size() == 8)
	{
		gprintf("CRC32 of %s is cached\n", gamePath);
		strncpy(crc_string, cachedCRC.c_str(), 8);
	}
	else
	{
		gprintf("Generating CRC32 for %s\n", gamePath);
		u32 buffer;
		ifstream infile;
		if(strstr(gameHeader.path, ".zip") != NULL)
		{
			infile.open(gameHeader.path, ios::binary);
			infile.seekg(0x0e, ios::beg);
			infile.read((char*)&buffer, 8);
			infile.close();
			strncpy(crc_string, fmt("%08x", (u32)__builtin_bswap32(buffer)), 8);
		}
		else if(strstr(gameHeader.path, ".7z") != NULL)
		{
			infile.open(gameHeader.path, ios::binary);
			infile.seekg(-8, ios::end);
			while(infile.tellg())
			{
				infile.read((char*)&buffer, 8);
				if(buffer == 0x00050111)
					break;
				infile.seekg(-9, ios::cur);
			}
			infile.seekg(-13, ios::cur);
			infile.read((char*)&buffer, 8);
			infile.close();
			strncpy(crc_string, fmt("%08x", (u32)__builtin_bswap32(buffer)), 8);
		}
		else
			strncpy(crc_string, fmt("%08x", crc32file(gameHeader.path)), 8);
		Checksums.setString("CHECKSUMS", gamePath, crc_string);
		Checksums.save();
	}
	url.replace(url.find(TAG_GAME_ID), strlen(TAG_GAME_ID), upperCase(crc_string).c_str());
	gprintf("URL: %s\n", url.c_str());
	return url;
}
