#ifndef CACHEDLIST
#define CACHEDLIST

#include "list.hpp"
#include "cache.hpp"
#include "config/config.hpp"
#include "gecko/gecko.h"

using namespace std;

enum {
	COVERFLOW_USB,
	COVERFLOW_DML,
	COVERFLOW_CHANNEL,
	COVERFLOW_EMU,
	COVERFLOW_HOMEBREW,
	COVERFLOW_MAX
};

class CachedList : public vector<dir_discHdr>
{
public:
	void Init(string cachedir, string settingsDir, string curLanguage, string DMLgameDir, bool extcheck, bool skipcheck)						/* Initialize Private Variables */
	{
		m_cacheDir = cachedir;
		m_settingsDir = settingsDir;
		m_curLanguage = m_lastLanguage = curLanguage;
		m_loaded = false;
		m_database = "";
		m_update = false;
		m_extcheck = extcheck;
		m_skipcheck = skipcheck;
		m_DMLgameDir = DMLgameDir;
		for(u32 i = 0; i < COVERFLOW_MAX; i++)
			force_update[i] = false;
	}

	void Update(u32 view = COVERFLOW_MAX)					/* Force db update on next load */
	{
		if(view == COVERFLOW_MAX)
			for(u32 i = 0; i < COVERFLOW_MAX; i++)
			{
				force_update[i] = true;
				gprintf("force_update[%d] = true\n", i);
			}
		else
		{
			force_update[view] = true;
			gprintf("force_update[%d] = true\n", view);
		}
	}

	void Load(string path, string containing, string m_lastLanguage, Config &m_plugin);
	void LoadChannels(string path, u32 channelType, string m_lastLanguage);

	void Unload(){if(m_loaded) {this->clear(); m_loaded = false; m_database = "";}};
	void Save() {if(m_loaded) CCache(*this, m_database, SAVE);}							/* Save All */

	void Get(dir_discHdr tmp, u32 index) {if(m_loaded) CCache(tmp, m_database, index, LOAD);}		/* Load One */
	void Set(dir_discHdr tmp, u32 index) {if(m_loaded) CCache(tmp, m_database, index, SAVE);}		/* Save One */

	void Add(dir_discHdr tmp) {if(m_loaded) CCache(*this, m_database, tmp, ADD);}					/* Add One */
	void Remove(u32 index) {if(m_loaded) CCache(*this, m_database, index, REMOVE);}		/* Remove One */

	void SetLanguage(string curLanguage) { m_curLanguage = curLanguage; }
private:
	string make_db_name(string path);

	bool m_loaded;
	bool m_update;
	bool m_wbfsFS;
	bool m_extcheck;
	bool m_skipcheck;
	u8 force_update[COVERFLOW_MAX];
	CList<dir_discHdr> list;
	string m_database;
	string m_cacheDir;
	string m_settingsDir;
	string m_curLanguage;
	string m_lastLanguage;
	string m_discinf;
	string m_DMLgameDir;
};

#endif
