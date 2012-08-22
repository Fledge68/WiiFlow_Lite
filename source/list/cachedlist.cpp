#include "cachedlist.hpp"
#include <typeinfo>

void CachedList::Load(string path, string containing, string m_lastLanguage, Config &m_plugin)													/* Load All */
{
	gprintf("\nLoading files containing %s in %s\n", containing.c_str(), path.c_str());
	m_loaded = false;
	m_database = sfmt("%s/%s.db", m_cacheDir.c_str(), (make_db_name(path)).c_str());

	m_wbfsFS = strncasecmp(DeviceHandler::Instance()->PathToFSName(path.c_str()), "WBFS", 4) == 0;
	
	bool update_games = false;
	bool update_homebrew = false;
	bool update_dml = false;
	bool update_emu = false;

	bool ditimes = false;
	if(!m_wbfsFS)
	{
		gprintf("Database file: %s\n", m_database.c_str());
		
		update_games = strcasestr(path.c_str(), "wbfs") != NULL && force_update[COVERFLOW_USB];
		update_homebrew = strcasestr(path.c_str(), "apps") != NULL && force_update[COVERFLOW_HOMEBREW];
		update_emu = strcasestr(path.c_str(), m_plugin.getString("PLUGIN","romDir","").c_str()) != NULL && force_update[COVERFLOW_EMU];

		const char* partition = DeviceName[DeviceHandler::Instance()->PathToDriveType(path.c_str())];
		update_dml = strcasestr(path.c_str(), fmt(strncmp(partition, "sd", 2) != 0 ? m_DMLgameDir.c_str() : "%s:/games", partition)) != NULL && force_update[COVERFLOW_DML];

		gprintf("update_games=%d update_homebrew=%d update_dml=%d, update_emu=%d\n", update_games, update_homebrew, update_dml, update_emu);
		if(update_games || update_homebrew || update_dml || update_emu)
			remove(m_database.c_str());

		m_discinf = sfmt("%s/disc.info", path.c_str());
		struct stat filestat, discinfo, cache;
		gprintf("%s\n", path.c_str());
		if(stat(path.c_str(), &filestat) == -1) 
			return;

		bool update_lang = m_lastLanguage != m_curLanguage;
		bool noDB = stat(m_database.c_str(), &cache) == -1;
		bool mtimes = filestat.st_mtime > cache.st_mtime;
		if(strcasestr(m_discinf.c_str(), "wbfs") != NULL && stat(m_discinf.c_str(), &discinfo) != -1)		
			ditimes = discinfo.st_mtime > cache.st_mtime;

		m_update = update_lang || noDB || (!m_skipcheck && (mtimes || ditimes));
		if(m_update) 
			gprintf("Cache of %s is being updated because:\n", path.c_str());
		if(update_lang) 
			gprintf("Languages are different!\nOld language string: %s\nNew language string: %s\n", m_lastLanguage.c_str(), m_curLanguage.c_str());
		if(noDB) 
			gprintf("A database was not found!\n");
		if(!m_skipcheck && (mtimes || ditimes)) 
			gprintf("The WBFS folder was modified!\nCache date: %i\nFolder date: %i\n", cache.st_mtime, filestat.st_mtime);

		if(m_extcheck && !m_update && !m_skipcheck)
		{
			bool m_chupdate = false;
			DIR *dir = opendir(path.c_str());
			struct dirent *entry;
			while((entry = readdir(dir)) != NULL)
			{
				m_discinf = sfmt("%s/%s", path.c_str(), entry->d_name);
				if(stat(m_discinf.c_str(), &discinfo) != -1)
					m_chupdate = discinfo.st_mtime > cache.st_mtime;
				if(m_chupdate)
					break;
			}
			m_update = m_chupdate;
		}
	}

	if(update_games) 
		force_update[COVERFLOW_USB] = false;
	if(update_homebrew) 
		force_update[COVERFLOW_HOMEBREW] = false;
	if(update_dml) 
		force_update[COVERFLOW_DML] = false;

	if(m_update || m_wbfsFS)
	{
		gprintf("Calling list to update filelist\n");
		
		vector<string> pathlist;
		list.GetPaths(pathlist, containing, path, m_wbfsFS, (update_dml || (m_update && strcasestr(path.c_str(), ":/games") != NULL)), !update_emu);
		list.GetHeaders(pathlist, *this, m_settingsDir, m_curLanguage, m_DMLgameDir, m_plugin);

		path.append("/touch.db");
		FILE *file = fopen(path.c_str(), "wb");
		fclose(file);
		remove(path.c_str());
		
		m_loaded = true;
		m_update = false;
		
		if(pathlist.size() > 0)
			Save();
		pathlist.clear();
	}
	else
	{
		CCache(*this, m_database, LOAD);
		m_loaded = true;
	}
}

void CachedList::LoadChannels(string path, u32 channelType, string m_lastLanguage)													/* Load All */
{
	m_loaded = false;
	m_update = true;

	bool emu = !path.empty();
	if(emu)
	{
		m_database = sfmt("%s/%s.db", m_cacheDir.c_str(), make_db_name(sfmt("%s/emu", path.c_str())).c_str());

		size_t find = m_database.find("//");
		if(find != string::npos)
			m_database.replace(find, 2, "/");

		if(force_update[COVERFLOW_CHANNEL])
			remove(m_database.c_str());

		//gprintf("%s\n", m_database.c_str());
		struct stat filestat, cache;

		string newpath = sfmt("%s%s", path.c_str(), "title");

		if(stat(newpath.c_str(), &filestat) == -1) return;

		m_update = force_update[COVERFLOW_CHANNEL] || m_lastLanguage != m_curLanguage || stat(m_database.c_str(), &cache) == -1 || filestat.st_mtime > cache.st_mtime;
	}

	force_update[COVERFLOW_CHANNEL] = false;

	if(m_update)
	{
		gprintf("Updating channels\n");
		list.GetChannels(*this, m_settingsDir, channelType, m_curLanguage);
		
		m_loaded = true;
		m_update = false;

		if(this->size() > 0 && emu) Save();
	}
	else
		CCache(*this, m_database, LOAD);

	m_loaded = true;
}

string CachedList::make_db_name(string path)
{
	string buffer = path;
	size_t find = buffer.find(":/");
	if(find != string::npos)
		buffer.replace(find, 2, "_");

	find = buffer.find("/");
	while(find != string::npos)
	{
		buffer[find] = '_';
		find = buffer.find("/");
	}
	return buffer;
}

