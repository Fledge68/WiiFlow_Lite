#include "cache.hpp"

CCache::CCache(vector<dir_discHdr> &list, string path, CMode mode)
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
		//gprintf("Fetching Item number %u in DB: %s\n", index, filename.c_str());
		fseek(cache, i * sizeof(dir_discHdr), SEEK_SET);
		fread((void *)&tmp, 1, sizeof(dir_discHdr), cache);
		//gprintf("Path %s\n", tmp.path);
		list.push_back(tmp);
	}
}
