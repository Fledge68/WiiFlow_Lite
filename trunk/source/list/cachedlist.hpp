#ifndef CACHEDLIST
#define CACHEDLIST

#include "list.hpp"
#include "cache.hpp"
#include "safe_vector.hpp"
#include "gecko.h"

using namespace std;

enum {
	COVERFLOW_USB,
	COVERFLOW_CHANNEL,
	COVERFLOW_HOMEBREW,
	COVERFLOW_DML,
	COVERFLOW_MAX
};

template <typename T = dir_discHdr>
class CachedList : public safe_vector<T>
{
  public:
	void Init(string cachedir, string settingsDir, string curLanguage, string DMLgameDir)						/* Initialize Private Variables */
	{
		m_cacheDir = cachedir;
		m_settingsDir = settingsDir;
		m_curLanguage = m_lastLanguage = curLanguage;
		m_loaded = false;
		m_database = "";
		m_update = false;
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

    void Load(string path, string containing, string m_lastLanguage);
	void LoadChannels(string path, u32 channelType, string m_lastLanguage);

    void Unload(){if(m_loaded) {this->clear(); m_loaded = false; m_database = "";}};
    void Save() {if(m_loaded) CCache<T>(*this, m_database, SAVE);}							/* Save All */

    void Get(T tmp, u32 index) {if(m_loaded) CCache<T>(tmp, m_database, index, LOAD);}		/* Load One */
    void Set(T tmp, u32 index) {if(m_loaded) CCache<T>(tmp, m_database, index, SAVE);}		/* Save One */

    void Add(T tmp) {if(m_loaded) CCache<T>(*this, m_database, tmp, ADD);}					/* Add One */
    void Remove(u32 index) {if(m_loaded) CCache<T>(*this, m_database, index, REMOVE);}		/* Remove One */
	
	void SetLanguage(string curLanguage) { m_curLanguage = curLanguage; }
  private:
    string make_db_name(string path);

    bool m_loaded;
    bool m_update;
    bool m_wbfsFS;
	u8 force_update[COVERFLOW_MAX];
    CList<T> list;
    string m_database;
    string m_cacheDir;
	string m_settingsDir;
	string m_curLanguage;
	string m_lastLanguage;
	string m_discinf;
	string m_DMLgameDir;
};

#endif
