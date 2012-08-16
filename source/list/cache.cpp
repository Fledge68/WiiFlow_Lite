#include "cache.hpp"

CCache::CCache(dir_discHdr &tmp, string path, u32 index, CMode mode) /* Load/Save One */
{
	filename = path;
	//gprintf("Openning DB: %s\n", filename.c_str());

	cache = fopen(filename.c_str(), io[mode]);
	if(!cache) return;

	switch(mode)
	{
		case LOAD:
			LoadOne(tmp, index);
			break;
		case SAVE:
			SaveOne(tmp, index);
			break;
		default:
			return;
	}
}

CCache::CCache(vector<dir_discHdr> &list, string path , CMode mode) /* Load/Save All */
{
	filename = path;
	//gprintf("Opening DB: %s\n", filename.c_str());

	cache = fopen(filename.c_str(), io[mode]);
	if(!cache) return;

	switch(mode)
	{
		case LOAD:
			LoadAll(list);
			break;
		case SAVE:
			SaveAll(list);
			break;
		default:
			return;
	}
}

CCache::CCache(vector<dir_discHdr> &list, string path, dir_discHdr tmp, CMode mode) /* Add One */
{
	filename = path;
	//gprintf("Openning DB: %s\n", filename.c_str());

	cache = fopen(filename.c_str(), io[mode]);
	if(!cache) return;

	switch(mode)
	{
		case ADD:
			AddOne(list, tmp);
			break;
		default:
			return;
	}
}

CCache::CCache(vector<dir_discHdr> &list, string path, u32 index, CMode mode)  /* Remove One */
{
	filename = path;
	//gprintf("Openning DB: %s\n", filename.c_str());

	cache = fopen(filename.c_str(), io[mode]);
	if(!cache) return;

	switch(mode)
	{
		case REMOVE:
			RemoveOne(list, index);
			break;
		default:
			return;
	}
}

CCache::~CCache()
{
	//gprintf("Closing DB: %s\n", filename.c_str());
	if(cache) fclose(cache);
	cache = NULL;
}

void CCache::SaveAll(vector<dir_discHdr> list)
{
	//gprintf("Updating DB: %s\n", filename.c_str());
	if(!cache) return;
	fwrite((void *)&list[0], 1, list.size() * sizeof(dir_discHdr), cache);
}

void CCache::SaveOne(dir_discHdr tmp, u32 index)
{
	//gprintf("Updating Item number %u in DB: %s\n", index, filename.c_str());
	if(!cache) return;
	fseek(cache, index * sizeof(dir_discHdr), SEEK_SET);
	fwrite((void *)&tmp, 1, sizeof(dir_discHdr), cache);
}

void CCache::LoadAll(vector<dir_discHdr> &list)
{
	if(!cache) return;

	//gprintf("Loading DB: %s\n", filename.c_str());

	dir_discHdr tmp;
	fseek(cache, 0, SEEK_END);
	u64 fileSize = ftell(cache);
	fseek(cache, 0, SEEK_SET);

	u32 count = (u32)(fileSize / sizeof(dir_discHdr));
	
	list.reserve(count + list.size());
	for(u32 i = 0; i < count; i++)
	{
		LoadOne(tmp, i);
		list.push_back(tmp);
	}
}

void CCache::LoadOne(dir_discHdr &tmp, u32 index)
{
	if(!cache) return;

	//gprintf("Fetching Item number %u in DB: %s\n", index, filename.c_str());
	fseek(cache, index * sizeof(dir_discHdr), SEEK_SET);
	fread((void *)&tmp, 1, sizeof(dir_discHdr), cache);
	//gprintf("Path %s\n", tmp.path);
}

void CCache::AddOne(vector<dir_discHdr> &list, dir_discHdr tmp)
{
	//gprintf("Adding Item number %u in DB: %s\n", list.size()+1, filename.c_str());
	list.push_back(tmp);

	if(!cache) return;
	fwrite((void *)&tmp, 1, sizeof(dir_discHdr), cache);  // FILE* is opened as "ab+" so its always written to the EOF.
}

void CCache::RemoveOne(vector<dir_discHdr> &list, u32 index)
{
	//gprintf("Removing Item number %u in DB: %s\n", index, filename.c_str());
	list.erase(list.begin() + index);
	SaveAll(list);
}
