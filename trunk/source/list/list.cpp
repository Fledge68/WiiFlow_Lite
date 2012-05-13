#include "list.hpp"
#include "gecko.h"
#include "GameTDB.hpp"
#include "config.hpp"
#include "defines.h"
#include "channels.h"
#include "gc.h"

template <typename T>
void CList<T>::GetPaths(vector<string> &pathlist, string containing, string directory, bool wbfs_fs, bool dml)
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
			if (ent->d_name[0] == '.') continue;
			//if (strlen(ent->d_name) < 6) continue;

			if(ent->d_type == DT_REG)
			{
				for(vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
					if (strcasestr(ent->d_name, (*compare).c_str()) != NULL)
					{
						//gprintf("Pushing %s to the list.\n", sfmt("%s/%s", directory.c_str(), ent->d_name).c_str());
						pathlist.push_back(sfmt("%s/%s", directory.c_str(), ent->d_name));
						break;
					}
			}
			else
				temp_pathlist.push_back(sfmt("%s/%s", directory.c_str(), ent->d_name));
		}
		closedir(dir_itr);

		if(temp_pathlist.size() > 0)
		{
			bool FoundDMLgame;
			for(vector<string>::iterator templist = temp_pathlist.begin(); templist != temp_pathlist.end(); templist++)
			{
				dir_itr = opendir((*templist).c_str());
				if (!dir_itr) continue;
				FoundDMLgame = false;

				/* Read secondary entries */
				while((ent = readdir(dir_itr)) != NULL)
				{
					if(ent->d_type == DT_REG && strlen(ent->d_name) > 7)
					{
						for(vector<string>::iterator compare = compares.begin(); compare != compares.end(); compare++)
						{
							if(strcasestr(ent->d_name, (*compare).c_str()) != NULL)
							{
								FoundDMLgame = true;
								//gprintf("Pushing %s to the list.\n", sfmt("%s/%s", (*templist).c_str(), ent->d_name).c_str());
								pathlist.push_back(sfmt("%s/%s", (*templist).c_str(), ent->d_name));
								break;
							}
						}
					}
					else if(dml && !FoundDMLgame && strncasecmp(ent->d_name, "sys", 3) == 0)
					{
						FILE *f;
						f = fopen(fmt("%s/%s/boot.bin", (*templist).c_str(), ent->d_name), "rb");
						if(f)
						{
							fclose(f);
							//gprintf("Pushing %s to the list.\n", sfmt("%s/%s/boot.bin", (*templist).c_str(), ent->d_name).c_str());
							pathlist.push_back(sfmt("%s/%s/boot.bin", (*templist).c_str(), ent->d_name));
							break;
						}
					}
				}
				closedir(dir_itr);
			}
		}
	}
	else
	{
		if(strcasestr(containing.c_str(), ".dol") != 0) return;

		int partition = DeviceHandler::Instance()->PathToDriveType(directory.c_str());
		wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
		if (!handle) return;

		u32 count = wbfs_count_discs(handle);
		for(u32 i = 0; i < count; i++)
			pathlist.push_back(directory);
	}
}

template <>
void CList<string>::GetHeaders(vector<string> pathlist, vector<string> &headerlist, string, string, string, Config&)
{
	//gprintf("Getting headers for CList<string>\n");

	if(pathlist.size() < 1) return;
	headerlist.reserve(pathlist.size() + headerlist.size());

	for(vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
		headerlist.push_back((*itr).c_str());
}

template <>
void CList<dir_discHdr>::GetHeaders(vector<string> pathlist, vector<dir_discHdr> &headerlist, string settingsDir, string curLanguage, string DMLgameUSBDir, Config &plugin)
{
	if(pathlist.size() < 1) return;
	headerlist.reserve(pathlist.size() + headerlist.size());

	gprintf("Getting headers for paths in pathlist (%d)\n", pathlist.size());

	dir_discHdr tmp;
	u32 count = 0;
	string GTitle;

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

	for(vector<string>::iterator itr = pathlist.begin(); itr != pathlist.end(); itr++)
	{
		bzero(&tmp, sizeof(dir_discHdr));
		strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));
		tmp.hdr.index = headerlist.size();
		tmp.hdr.casecolor = 1;
		
		bool wbfs = (*itr).rfind(".wbfs") != string::npos || (*itr).rfind(".WBFS") != string::npos;
		if (wbfs || (*itr).rfind(".iso")  != string::npos || (*itr).rfind(".ISO")  != string::npos
				 || (*itr).rfind(".bin")  != string::npos || (*itr).rfind(".BIN")  != string::npos)
		{
			char* filename = &(*itr)[(*itr).find_last_of('/')+1];
			const char* dml_partition = DeviceName[DeviceHandler::Instance()->PathToDriveType((*itr).c_str())];
			if((strcasecmp(filename, "game.iso") == 0 || strcasecmp(filename, "boot.bin") == 0) && strstr((*itr).c_str(), sfmt((strncmp(dml_partition, "sd", 2) != 0) ? DMLgameUSBDir.c_str() : DML_DIR, dml_partition).c_str()) != NULL)
			{
				FILE *fp = fopen((*itr).c_str(), "rb");
				if( fp )
				{
					fseek( fp, 0, SEEK_SET );
					fread( tmp.hdr.id, 1, 6, fp );

					u8 gc_disc[1];

					fread(gc_disc, 1, 1, fp);

					GTitle = custom_titles.getString( "TITLES", (const char *) tmp.hdr.id );
					int ccolor = custom_titles.getColor( "COVERS", (const char *) tmp.hdr.id, tmp.hdr.casecolor ).intVal();

					if( GTitle.size() > 0 || ( gameTDB.IsLoaded() && gameTDB.GetTitle( (char *)tmp.hdr.id, GTitle ) ) )
					{
						//mbstowcs( tmp.title, GTitle.c_str(), sizeof(tmp.title) );
						//Asciify( tmp.title );
						wstringEx tmpString;
						tmpString.fromUTF8(GTitle.c_str());
						wcsncpy(tmp.title, tmpString.c_str(), 64);
						if(gc_disc[0])
							wcslcat(tmp.title, L" disc 2", sizeof(tmp.title));

						tmp.hdr.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor( (char *)tmp.hdr.id );
						if( tmp.hdr.casecolor == 0xffffffff )
							tmp.hdr.casecolor = 0;

						tmp.hdr.gc_magic = 0xc2339f3d;
						(*itr)[(*itr).find_last_of('/')] = 0;
						if(strcasecmp(filename, "boot.bin") == 0)
							(*itr)[(*itr).find_last_of('/')] = 0;
						(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
						strcpy( tmp.path, (*itr).c_str() );

						if(strncmp(dml_partition, "sd", 2) != 0)
						{
							if (GC_GameIsInstalled((char*)tmp.hdr.id, DeviceName[SD], DML_DIR) || GC_GameIsInstalled((char*)fmt("%s [%s]", tmp.hdr.title, (char *)tmp.hdr.id), DeviceName[SD], DML_DIR) || GC_GameIsInstalled(tmp.path, DeviceName[SD], DML_DIR))
								wcslcat(tmp.title, L" \n(on SD)", sizeof(tmp.title));
						}

						gprintf("Found: %s\n", tmp.path);
						headerlist.push_back( tmp );
						continue;
					}

					fseek( fp, 0, SEEK_SET );
					fread( &tmp.hdr, sizeof( discHdr ), 1, fp);
					fclose(fp);
					
					if ( tmp.hdr.gc_magic == 0xc2339f3d )
					{
						//mbstowcs( tmp.title, (const char *)tmp.hdr.title, sizeof( tmp.hdr.title ) );
						//Asciify(tmp.title);
						wstringEx tmpString;
						tmpString.fromUTF8((const char *)tmp.hdr.title);
						wcsncpy(tmp.title, tmpString.c_str(), 64);
						if(gc_disc[0])
							wcslcat(tmp.title, L" disc 2", sizeof(tmp.title));

						tmp.hdr.casecolor = 0;
						(*itr)[(*itr).find_last_of('/')] = 0;
						if(strcasecmp(filename, "boot.bin") == 0)
							(*itr)[(*itr).find_last_of('/')] = 0;
						(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
						strcpy( tmp.path, (*itr).c_str() );

						if(strncmp(dml_partition, "sd", 2) != 0)
						{
							if (GC_GameIsInstalled((char*)tmp.hdr.id, DeviceName[SD], DML_DIR) || GC_GameIsInstalled((char*)fmt("%s [%s]", tmp.hdr.title, (char *)tmp.hdr.id), DeviceName[SD], DML_DIR) || GC_GameIsInstalled(tmp.path, DeviceName[SD], DML_DIR))
								wcslcat(tmp.title, L" \n(on SD)", sizeof(tmp.title));
						}

						gprintf("Found: %s\n", tmp.path);
						headerlist.push_back( tmp );
						continue;
					}
				}
				continue;
			}

			Check_For_ID(tmp.hdr.id, (*itr).c_str(), "[", "]"); 	 			/* [GAMEID] Title, [GAMEID]_Title, Title [GAMEID], Title_[GAMEID] */
			if(tmp.hdr.id[0] == 0)
			{
				Check_For_ID(tmp.hdr.id, (*itr).c_str(), "/", "."); 			/* GAMEID.wbfs, GAMEID.iso */
				if(tmp.hdr.id[0] == 0)
				{
					Check_For_ID(tmp.hdr.id, (*itr).c_str(), "/", "_"); 		/* GAMEID_Title */
					if(tmp.hdr.id[0] == 0)
					{
						Check_For_ID(tmp.hdr.id, (*itr).c_str(), "_", "."); 	/* Title_GAMEID */ // <-- Unsafe?
						if(tmp.hdr.id[0] == 0)
							Check_For_ID(tmp.hdr.id, (*itr).c_str(), " ", "."); /* Title GAMEID */ //<-- Unsafe?
					}
				}
			}

			if (!isalnum(tmp.hdr.id[0]) || tmp.hdr.id[0] == 0 || memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) == 0)
			{
				gprintf("Skipping file: '%s'\n", (*itr).c_str());
				continue;
			}

			// Get info from custom titles
			GTitle = custom_titles.getString("TITLES", (const char *) tmp.hdr.id);
			int ccolor = custom_titles.getColor("COVERS", (const char *) tmp.hdr.id, tmp.hdr.casecolor).intVal();
				
			if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle((char *)tmp.hdr.id, GTitle)))
			{
				//mbstowcs(tmp.title, GTitle.c_str(), sizeof(tmp.title));
				//Asciify(tmp.title);
				wstringEx tmpString;
				tmpString.fromUTF8(GTitle.c_str());
				wcsncpy(tmp.title, tmpString.c_str(), 64);
				tmp.hdr.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor((char *)tmp.hdr.id);

				tmp.hdr.wifi = gameTDB.GetWifiPlayers((char *)tmp.hdr.id);
				tmp.hdr.players = gameTDB.GetPlayers((char *)tmp.hdr.id);
				//tmp.hdr.controllers = gameTDB.GetAccessories((char *)tmp.hdr.id);

				tmp.hdr.magic = 0x5D1C9EA3;
				headerlist.push_back(tmp);
				continue;
			}

			FILE *fp = fopen((*itr).c_str(), "rb");
			if (fp)
			{
				fseek(fp, wbfs ? 512 : 0, SEEK_SET);
				fread(&tmp.hdr, sizeof(discHdr), 1, fp);
				fclose(fp);
			}

			if (tmp.hdr.magic == 0x5D1C9EA3)
			{
				//mbstowcs(tmp.title, (const char *)tmp.hdr.title, sizeof(tmp.hdr.title));
				//Asciify(tmp.title);
				wstringEx tmpString;
				tmpString.fromUTF8((const char *)tmp.hdr.title);
				wcsncpy(tmp.title, tmpString.c_str(), 64);
				tmp.hdr.casecolor = ccolor != 1 ? ccolor : 1;
				headerlist.push_back(tmp);
				continue;
			}
			
			/*if(wbfs)
			{
				wbfs_t *part = WBFS_Ext_OpenPart((char *)(*itr).c_str());
				if (!part) continue;

				// Get header 
				if(wbfs_get_disc_info(part, 0, (u8*)&tmp.hdr, sizeof(discHdr), NULL) == 0
				&& memcmp(tmp.hdr.id, "__CFG_", sizeof tmp.hdr.id) != 0)
				{
					mbstowcs(tmp.title, (const char *)tmp.hdr.title, sizeof(tmp.title));
					Asciify(tmp.title);
					
					headerlist.push_back(tmp);
				}
				WBFS_Ext_ClosePart(part);
				continue;
			}*/
		}
		else if((*itr).rfind(".dol") != string::npos || (*itr).rfind(".DOL") != string::npos
			|| (*itr).rfind(".elf") != string::npos || (*itr).rfind(".ELF")  != string::npos)
		{
			char *filename = &(*itr)[(*itr).find_last_of('/')+1];

			if(strcasecmp(filename, "boot.dol") != 0 && strcasecmp(filename, "boot.elf") != 0) continue;

			(*itr)[(*itr).find_last_of('/')] = 0;
			(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
			
			(*itr)[0] = toupper((*itr)[0]);
			for (u32 i = 1; i < (*itr).size(); ++i)
			{
				if((*itr)[i] == '_' || (*itr)[i] == '-')
					(*itr)[i] = ' ';

				if((*itr)[i] == ' ')
				{
					(*itr)[i + 1] = toupper((*itr)[i + 1]);
					i++;
				}
				else (*itr)[i] = tolower((*itr)[i]);
			}

			memcpy(tmp.hdr.id, (*itr).c_str(), 6);
			for (u32 i = 0; i < 6; ++i)
			{
				tmp.hdr.id[i] = toupper(tmp.hdr.id[i]);
				if(!isalnum(tmp.hdr.id[i]) || tmp.hdr.id[i] == ' ' || tmp.hdr.id[i] == '\0')
					tmp.hdr.id[i] = '_';
			}

			// Get info from custom titles
			wstringEx tmpString;
			GTitle = custom_titles.getString("TITLES", (const char *) tmp.hdr.id);
			int ccolor = custom_titles.getColor("COVERS", (const char *) tmp.hdr.id, tmp.hdr.casecolor).intVal();			
			if(GTitle.size() > 0 || (gameTDB.GetTitle((char *)tmp.hdr.id, GTitle)))
			{
				//mbstowcs(tmp.title, GTitle.c_str(), sizeof(tmp.title));
				tmpString.fromUTF8(GTitle.c_str());
				tmp.hdr.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor((char *)tmp.hdr.id);
			}
			else
			{
				//mbstowcs(tmp.title, (*itr).c_str(), sizeof(tmp.title));
				tmpString.fromUTF8((*itr).c_str());
				tmp.hdr.casecolor = ccolor != 1 ? ccolor : 1;
			}
			//Asciify(tmp.title);
			wcsncpy(tmp.title, tmpString.c_str(), 64);
			headerlist.push_back(tmp);
			continue;
		}
		else if(strncasecmp(DeviceHandler::Instance()->PathToFSName((*itr).c_str()), "WBFS", 4) == 0)
		{
			u8 partition = DeviceHandler::Instance()->PathToDriveType((*itr).c_str());
			wbfs_t* handle = DeviceHandler::Instance()->GetWbfsHandle(partition);
			if (!handle)
				return;

			s32 ret = wbfs_get_disc_info(handle, count, (u8 *)&tmp.hdr, sizeof(struct discHdr), NULL);
			count++;
			if(ret != 0) 
				continue;

			GTitle = custom_titles.getString("TITLES", (const char *) tmp.hdr.id);
			int ccolor = custom_titles.getColor("COVERS", (const char *) tmp.hdr.id, 1).intVal();

			wstringEx tmpString;
			if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle((char *)tmp.hdr.id, GTitle)))
			{
				//mbstowcs(tmp.title, GTitle.c_str(), sizeof(tmp.title));
				tmpString.fromUTF8(GTitle.c_str());
				tmp.hdr.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor((char *)tmp.hdr.id);
				tmp.hdr.wifi = gameTDB.GetWifiPlayers((char *)tmp.hdr.id);
				tmp.hdr.players = gameTDB.GetPlayers((char *)tmp.hdr.id);

				//tmp.hdr.controllers = gameTDB.GetAccessories((char *)tmp.hdr.id);
				if (tmp.hdr.magic == 0x5D1C9EA3)
				{
					//Asciify(tmp.title);
					wcsncpy(tmp.title, tmpString.c_str(), 64);
					headerlist.push_back(tmp);
				}
				continue;
			}

			if (tmp.hdr.magic == 0x5D1C9EA3)
			{
				//mbstowcs(tmp.title, (const char *)tmp.hdr.title, sizeof(tmp.hdr.title));
				//Asciify(tmp.title);
				tmpString.fromUTF8((const char *)tmp.hdr.title);
				wcsncpy(tmp.title, tmpString.c_str(), 64);
				tmp.hdr.casecolor = ccolor != 1 ? ccolor : 1;
				headerlist.push_back(tmp);
			}
			continue;
		}
		else if(plugin.loaded())
		{
			vector<string> types = plugin.getStrings("PLUGIN","fileTypes",'|');
			if (types.size() > 0)
			{
				for(vector<string>::iterator type_itr = types.begin(); type_itr != types.end(); type_itr++)
				{
					if(lowerCase(*itr).rfind((*type_itr).c_str()) != string::npos)
					{
						strncpy(tmp.path, (*itr).c_str(), sizeof(tmp.path));

						int plugin_ccolor;
						sscanf(plugin.getString("PLUGIN","coverColor","").c_str(), "%08x", &plugin_ccolor);
						int ccolor = custom_titles.getColor("COVERS", (const char *) tmp.hdr.id, plugin_ccolor).intVal();
						tmp.hdr.casecolor = ccolor != plugin_ccolor ? ccolor : plugin_ccolor;

						char tempname[64];
						(*itr).assign(&(*itr)[(*itr).find_last_of('/') + 1]);
						if((*itr).find_last_of('.') != string::npos)
							(*itr).erase((*itr).find_last_of('.'), (*itr).size() - (*itr).find_last_of('.'));
						strncpy(tempname, (*itr).c_str(), sizeof(tempname));
						//mbstowcs(tmp.title, tempname, sizeof(tmp.title));
						//Asciify(tmp.title);
						wstringEx tmpString;
						tmpString.fromUTF8(tempname);
						wcsncpy(tmp.title, tmpString.c_str(), 64);

						gprintf("Found: %s\n", tmp.path);
						sscanf(plugin.getString("PLUGIN","magic","").c_str(), "%08x", &tmp.hdr.magic); //Plugin magic
						tmp.hdr.gc_magic = 0x4c4f4c4f; //Abusing gc_magic for general emu detection ;)
						headerlist.push_back(tmp);
						break;
					}
				}
			}
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
		
		if (chan->id == NULL) continue; // Skip invalid channels

		dir_discHdr tmp;
		bzero(&tmp, sizeof(dir_discHdr));
		tmp.hdr.index = headerlist.size();
		tmp.hdr.casecolor = 1;

		memcpy(tmp.hdr.id, chan->id, 4);
		memcpy(tmp.title, chan->name, sizeof(tmp.title));
		string GTitle = custom_titles.getString("TITLES", (const char *) tmp.hdr.id);
		int ccolor = custom_titles.getColor("COVERS", (const char *) tmp.hdr.id, tmp.hdr.casecolor).intVal();

		if(GTitle.size() > 0 || (gameTDB.IsLoaded() && gameTDB.GetTitle((char *)tmp.hdr.id, GTitle)))
			mbstowcs(tmp.title, GTitle.c_str(), sizeof(tmp.title));

		Asciify(tmp.title);

		tmp.hdr.casecolor = ccolor != 1 ? ccolor : gameTDB.GetCaseColor((char *)tmp.hdr.id);

		tmp.hdr.wifi = gameTDB.GetWifiPlayers((char *)tmp.hdr.id);
		tmp.hdr.players = gameTDB.GetPlayers((char *)tmp.hdr.id);
		//tmp.hdr.controllers = gameTDB.GetAccessories((char *)tmp.hdr.id);
				
		tmp.hdr.chantitle = chan->title;
	
		headerlist.push_back(tmp);
	}

	if(gameTDB.IsLoaded())
		gameTDB.CloseFile();
}

template <typename T>
void CList<T>::Check_For_ID(u8 *id, string path, string one, string two)
{
	size_t idstart = path.find_last_of(one);
	size_t idend = path.find_last_of(two);
	if (idend != string::npos && idstart != string::npos && idend - idstart == 7)
		for(u8 pos = 0; pos < 6; pos++)
			id[pos] = toupper(path[idstart + 1 + pos]);
}

template class CList<dir_discHdr>;
template class CList<string>;
