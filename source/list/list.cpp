#include "list.hpp"
#include "types.h"
#include "channel/channels.h"
#include "config/config.hpp"
#include "fileOps/fileOps.h"
#include "gecko/gecko.h"
#include "gc/gc.hpp"
#include "gui/GameTDB.hpp"

template <typename T>
void CList<T>::GetPaths(vector<string> &pathlist, string containing, string directory, bool wbfs_fs, bool dml, bool depth_limit)
{
	if (!wbfs_fs)
	{
		/* Open primary directory */
		DIR *dir_itr = opendir(directory.c_str());
		if (!dir_itr) return;

		vector<string> compares = stringToVector(containing, '|');
		vector<string> temp_pathlist;

		struct dirent *ent;

		/* Read primary entries */
		while((ent = readdir(dir_itr)) != NULL)
		{
			if(ent->d_name[0] == '.')
				continue;

			if(ent->d_type == DT_REG)
			{
				for(vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
				{
					if (strcasestr(ent->d_name, (*compare).c_str()) != NULL)
					{
						//gprintf("Pushing %s to the list.\n", sfmt("%s/%s", directory.c_str(), ent->d_name).c_str());
						pathlist.push_back(sfmt("%s/%s", directory.c_str(), ent->d_name));
						break;
					}
				}
			}
			else
				temp_pathlist.push_back(sfmt("%s/%s", directory.c_str(), ent->d_name));
		}
		closedir(dir_itr);

		while(temp_pathlist.size())
		{
			while((dir_itr = opendir(temp_pathlist[0].c_str())) && !dir_itr)
				temp_pathlist.erase(temp_pathlist.begin());

			/* Read subdirectory */
			while((ent = readdir(dir_itr)) != NULL)
			{
				if(ent->d_name[0] == '.')
					continue;
				if(ent->d_type == DT_REG)
				{
					for(vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
					{
						if(strcasestr(ent->d_name, (*compare).c_str()) != NULL)
						{
							//gprintf("Pushing %s to the list.\n", sfmt("%s/%s", temp_pathlist[0].c_str(), ent->d_name).c_str());
							pathlist.push_back(sfmt("%s/%s", temp_pathlist[0].c_str(), ent->d_name));
							break;
						}
					}
				}
				else
				{
					if(!depth_limit)
						temp_pathlist.push_back(sfmt("%s/%s", temp_pathlist[0].c_str(), ent->d_name));
					else if(dml && strncasecmp(ent->d_name, "sys", 3) == 0 && fsop_DirExist(fmt("%s/root", temp_pathlist[0].c_str())))
					{
						//gprintf("Pushing %s to the list.\n", sfmt("%s/%s/boot.bin", temp_pathlist[0].c_str(), ent->d_name).c_str());
						pathlist.push_back(sfmt("%s/%s/boot.bin", temp_pathlist[0].c_str(), ent->d_name));
					}
				}
			}
			closedir(dir_itr);
			/* Finished reading subdirectory, erase it */
			temp_pathlist.erase(temp_pathlist.begin());
		}
	}
	else
	{
		if(strcasestr(containing.c_str(), ".dol") != 0)
			return;

		int partition = DeviceHandler::Instance()->PathToDriveType(directory.c_str());
		wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
		if(!handle)
			return;

		u32 count = wbfs_count_discs(handle);
		for(u32 i = 0; i < count; i++)
			pathlist.push_back(directory);
	}
}

template <>
void CList<dir_discHdr>::GetHeaders(vector<string> pathlist, vector<dir_discHdr> &headerlist, string settingsDir, string curLanguage, string DMLgameUSBDir, Config &plugin)
{
	if(pathlist.size() < 1)
		return;

	headerlist.reserve(pathlist.size() + headerlist.size());
	gprintf("Getting headers for paths in pathlist (%d)\n", pathlist.size());

	vector<char*> GC_SD_IDs;
	bool GC_SD_IDs_loaded = false;

	discHdr gc_hdr;
	dir_discHdr tmp;
	u32 count = 0;
	string GTitle;

	Config custom_titles;
	if(settingsDir.size() > 0)
	{
		string custom_titles_path = sfmt("%s/" CTITLES_FILENAME, settingsDir.c_str());
		custom_titles.load(custom_titles_path.c_str());
	}

	GameTDB gameTDB;
	if(settingsDir.size() > 0)
	{
		gameTDB.OpenFile(fmt("%s/wiitdb.xml", settingsDir.c_str()));
		if(curLanguage.size() == 0) curLanguage = "EN";
		gameTDB.SetLanguageCode(curLanguage.c_str());
	}

	for(vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
	{
		memset(&tmp, 0, sizeof(tmp));
		strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));
		tmp.index = headerlist.size();
		tmp.casecolor = 1;

		bool wbfs = (*itr).rfind(".wbfs") != string::npos || (*itr).rfind(".WBFS") != string::npos;

		if(plugin.loaded())
		{
			vector<string> types = plugin.getStrings("PLUGIN","fileTypes",'|');
			if (types.size() > 0)
			{
				for(vector<string>::iterator type_itr = types.begin(); type_itr != types.end(); type_itr++)
				{
					if(lowerCase(*itr).rfind((*type_itr).c_str()) != string::npos)
					{
						strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));
						strncpy(tmp.id, "PLUGIN", sizeof(tmp.id));
						sscanf(plugin.getString("PLUGIN","coverColor","").c_str(), "%08x", &tmp.casecolor);

						char tempname[64];
						(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
						if((*itr).find_last_of('.') != string::npos)
							(*itr).erase((*itr).find_last_of('.'), (*itr).size() - (*itr).find_last_of('.'));
						strncpy(tempname, (*itr).c_str(), sizeof(tempname));

						wstringEx tmpString;
						tmpString.fromUTF8(tempname);
						wcsncpy(tmp.title, tmpString.c_str(), 64);
						Asciify(tmp.title);

						//gprintf("Found: %ls\n", tmp.title);
						sscanf(plugin.getString("PLUGIN","magic","").c_str(), "%08x", &tmp.settings[0]); //Plugin magic
						tmp.type = TYPE_PLUGIN;
						headerlist.push_back(tmp);
						break;
					}
				}
			}
			continue;
		}
		else if(wbfs || (*itr).rfind(".iso")  != string::npos || (*itr).rfind(".ISO")  != string::npos
				 || (*itr).rfind(".bin")  != string::npos || (*itr).rfind(".BIN")  != string::npos)
		{
			char* filename = &(*itr)[(*itr).find_last_of('/')+1];
			const char* dml_partition = DeviceName[DeviceHandler::Instance()->PathToDriveType((*itr).c_str())];
			if(strcasecmp(filename, "game.iso") == 0 || strcasecmp(filename, "gam1.iso") == 0 || strcasecmp(filename, "boot.bin") == 0)
			{
				FILE *fp = fopen((*itr).c_str(), "rb");
				if(fp)
				{
					u8 gc_disc[1];
					fseek(fp, 6, SEEK_SET);
					fread(gc_disc, 1, 1, fp);

					memset(&gc_hdr, 0, sizeof(discHdr));
					fseek(fp, 0, SEEK_SET);
					fread(&gc_hdr, sizeof(discHdr), 1, fp);
					fclose(fp);

					if(gc_hdr.gc_magic == GC_MAGIC)
					{
						strcpy(tmp.path, (*itr).c_str());
						strncpy(tmp.id, (char*)gc_hdr.id, 6);
						(*itr)[(*itr).find_last_of('/')] = 0;
						if(strcasecmp(filename, "boot.bin") == 0)
							(*itr)[(*itr).find_last_of('/')] = 0;
						(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
						GTitle = custom_titles.getString("TITLES", tmp.id);
						tmp.casecolor = 0;
						int ccolor = custom_titles.getColor("COVERS", tmp.id, tmp.casecolor).intVal();

						wstringEx tmpString;
						if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(tmp.id, GTitle)))
						{
							tmpString.fromUTF8(GTitle.c_str());
							tmp.casecolor = ccolor != 0 ? ccolor : gameTDB.GetCaseColor(tmp.id);
						}
						else
						{
							tmpString.fromUTF8(gc_hdr.title);
							tmp.casecolor = ccolor;
						}
						wcsncpy(tmp.title, tmpString.c_str(), 64);
						if(gc_disc[0])
							wcslcat(tmp.title, L" disc 2", sizeof(tmp.title));
						if(strncmp(dml_partition, "sd", 2) != 0)
						{
							if(!GC_SD_IDs_loaded)
							{
								CList<dir_discHdr> tmplist;
								vector<string> pathlist;
								tmplist.GetPaths(pathlist, ".iso|.bin", "sd:/games", false, true);
								vector<dir_discHdr> tmpGameList;
								tmplist.GetHeaders(pathlist, tmpGameList, settingsDir, curLanguage, DMLgameUSBDir, plugin);
								for(u8 i = 0; i < tmpGameList.size(); i++)
									GC_SD_IDs.push_back(tmpGameList.at(i).id);
								GC_SD_IDs_loaded = true;
							}
							tmp.settings[0] = 0;
							for(u8 i = 0; i < GC_SD_IDs.size(); i++)
							{
								if(strncasecmp(GC_SD_IDs.at(i), tmp.id, 6) == 0)
								{
									tmp.settings[0] = 1; //Later Checks can use this as easy information
									wcslcat(tmp.title, L" \n(on SD)", sizeof(tmp.title));
									break;
								}
							}
						}
						Asciify(tmp.title);

						//gprintf("Found: %ls\n", tmp.title);
						tmp.type = TYPE_GC_GAME;
						headerlist.push_back(tmp);
						continue;
					}
				}
				continue;
			}

			Check_For_ID(tmp.id, (*itr).c_str(), "[", "]"); 	 			/* [GAMEID] Title, [GAMEID]_Title, Title [GAMEID], Title_[GAMEID] */
			if(tmp.id[0] == 0)
			{
				Check_For_ID(tmp.id, (*itr).c_str(), "/", "."); 			/* GAMEID.wbfs, GAMEID.iso */
				if(tmp.id[0] == 0)
				{
					Check_For_ID(tmp.id, (*itr).c_str(), "/", "_"); 		/* GAMEID_Title */
					if(tmp.id[0] == 0)
					{
						Check_For_ID(tmp.id, (*itr).c_str(), "_", "."); 	/* Title_GAMEID */ // <-- Unsafe?
						if(tmp.id[0] == 0)
							Check_For_ID(tmp.id, (*itr).c_str(), " ", "."); /* Title GAMEID */ //<-- Unsafe?
					}
				}
			}

			if(!isalnum(tmp.id[0]) || tmp.id[0] == 0 || memcmp(tmp.id, "__CFG_", sizeof tmp.id) == 0)
			{
				gprintf("Skipping file: '%s'\n", (*itr).c_str());
				continue;
			}

			discHdr wii_hdr;
			FILE *fp = fopen((*itr).c_str(), "rb");
			if(fp)
			{
				fseek(fp, wbfs ? 512 : 0, SEEK_SET);
				fread(&wii_hdr, sizeof(discHdr), 1, fp);
				fclose(fp);
			}

			if(wii_hdr.magic == WII_MAGIC)
			{
				strncpy(tmp.id, (char*)wii_hdr.id, 6);
				GTitle = custom_titles.getString("TITLES", tmp.id);
				int ccolor = custom_titles.getColor("COVERS", tmp.id, tmp.casecolor).intVal();

				wstringEx tmpString;
				if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(tmp.id, GTitle)))
				{
					tmpString.fromUTF8(GTitle.c_str());
					tmp.wifi = gameTDB.GetWifiPlayers(tmp.id);
					tmp.players = gameTDB.GetPlayers(tmp.id);
					tmp.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(tmp.id);
				}
				else
				{
					tmpString.fromUTF8(wii_hdr.title);
					tmp.casecolor = ccolor;
				}
				wcsncpy(tmp.title, tmpString.c_str(), 64);
				Asciify(tmp.title);

				//gprintf("Found: %ls\n", tmp.title);
				tmp.type = TYPE_WII_GAME;
				headerlist.push_back(tmp);
				continue;
			}
		}
		else if(strncasecmp(DeviceHandler::Instance()->PathToFSName((*itr).c_str()), "WBFS", 4) == 0)
		{
			u8 partition = DeviceHandler::Instance()->PathToDriveType((*itr).c_str());
			wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
			if(!handle)
				return;

			discHdr wbfs_hdr;
			s32 ret = wbfs_get_disc_info(handle, count, (u8 *)&wbfs_hdr, sizeof(struct discHdr), NULL);
			count++;
			if(ret != 0) 
				continue;

			if(wbfs_hdr.magic == WII_MAGIC)
			{
				strncpy(tmp.id, (char*)wbfs_hdr.id, 6);
				GTitle = custom_titles.getString("TITLES", tmp.id);
				int ccolor = custom_titles.getColor("COVERS", tmp.id, tmp.casecolor).intVal();

				wstringEx tmpString;
				if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(tmp.id, GTitle)))
				{
					tmpString.fromUTF8(GTitle.c_str());
					tmp.wifi = gameTDB.GetWifiPlayers(tmp.id);
					tmp.players = gameTDB.GetPlayers(tmp.id);
					tmp.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(tmp.id);
				}
				else
				{
					tmpString.fromUTF8((const char *)wbfs_hdr.title);
					tmp.casecolor = ccolor;
				}
				wcsncpy(tmp.title, tmpString.c_str(), 64);
				Asciify(tmp.title);

				//gprintf("Found: %ls\n", tmp.title);
				tmp.type = TYPE_WII_GAME;
				headerlist.push_back(tmp);
			}
			continue;
		}
		else if((*itr).rfind(".dol") != string::npos || (*itr).rfind(".DOL") != string::npos
			|| (*itr).rfind(".elf") != string::npos || (*itr).rfind(".ELF")  != string::npos)
		{
			char *filename = &(*itr)[(*itr).find_last_of('/')+1];
			if(strcasecmp(filename, "boot.dol") != 0 && strcasecmp(filename, "boot.elf") != 0)
				continue;

			strncpy(tmp.id, "HB_APP", sizeof(tmp.id));

			(*itr)[(*itr).find_last_of('/')] = 0;
			strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));

			(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
			char homebrewtitle[64];
			strncpy(homebrewtitle, (*itr).c_str(), sizeof(homebrewtitle));

			tmp.casecolor = custom_titles.getColor("COVERS", homebrewtitle, tmp.casecolor).intVal();

			wstringEx tmpString;
			GTitle = custom_titles.getString("TITLES", homebrewtitle);		
			if(GTitle.size() > 0)
				tmpString.fromUTF8(GTitle.c_str());
			else
				tmpString.fromUTF8(homebrewtitle);
			wcsncpy(tmp.title, tmpString.c_str(), 64);
			Asciify(tmp.title);

			//gprintf("Found: %ls\n", tmp.title);
			tmp.type = TYPE_HOMEBREW;
			headerlist.push_back(tmp);
			continue;
		}
	}

	if(gameTDB.IsLoaded())
		gameTDB.CloseFile();
}

template <>
void CList<dir_discHdr>::GetChannels(vector<dir_discHdr> &headerlist, string settingsDir, u32 channelType, string curLanguage)
{
	Channels m_channels;
	m_channels.Init(channelType, curLanguage, true);

	Config custom_titles;
	if (settingsDir.size() > 0)
	{
		string custom_titles_path = sfmt("%s/" CTITLES_FILENAME, settingsDir.c_str());
		custom_titles.load(custom_titles_path.c_str());
	}

	GameTDB gameTDB;
	if (settingsDir.size() > 0)
	{
		gameTDB.OpenFile(fmt("%s/wiitdb.xml", settingsDir.c_str()));
		if(curLanguage.size() == 0) curLanguage = "EN";
		gameTDB.SetLanguageCode(curLanguage.c_str());
	}

	u32 count = m_channels.Count();

	headerlist.reserve(count);

	for (u32 i = 0; i < count; ++i)
	{
		Channel *chan = m_channels.GetChannel(i);
		
		if(chan->id == NULL) 
			continue; // Skip invalid channels

		dir_discHdr tmp;
		bzero(&tmp, sizeof(dir_discHdr));
		tmp.index = headerlist.size();
		tmp.casecolor = 1;

		tmp.settings[0] = TITLE_UPPER(chan->title);
		tmp.settings[1] = TITLE_LOWER(chan->title);
		strncpy(tmp.id, chan->id, 4);
		int ccolor = custom_titles.getColor("COVERS", tmp.id, tmp.casecolor).intVal();

		wstringEx tmpString;
		string GTitle = custom_titles.getString("TITLES", tmp.id);
		if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle(tmp.id, GTitle)))
		{
			tmpString.fromUTF8(GTitle.c_str());
			tmp.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor(tmp.id);
			tmp.wifi = gameTDB.GetWifiPlayers(tmp.id);
			tmp.players = gameTDB.GetPlayers(tmp.id);
		}
		else
		{
			tmpString = chan->name;
			tmp.casecolor = ccolor;
		}

		wcsncpy(tmp.title, tmpString.c_str(), 64);
		Asciify(tmp.title);
		//gprintf("Found: %ls\n", tmp.title);
		tmp.type = TYPE_CHANNEL;
		headerlist.push_back(tmp);
	}

	if(gameTDB.IsLoaded())
		gameTDB.CloseFile();
}

template <typename T>
void CList<T>::Check_For_ID(char *id, string path, string one, string two)
{
	memset(id, 0, sizeof(id));
	size_t idstart = path.find_last_of(one);
	size_t idend = path.find_last_of(two);
	if (idend != string::npos && idstart != string::npos && idend - idstart == 7)
		for(u8 pos = 0; pos < 6; pos++)
			id[pos] = toupper(path[idstart + 1 + pos]);
}

template class CList<dir_discHdr>;
template class CList<string>;
