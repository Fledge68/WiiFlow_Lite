#include "cache.hpp"

template <typename T>
CCache<T>::CCache(T &tmp, string path, u32 index, CMode mode) /* Load/Save One */
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

template <typename T>
CCache<T>::CCache(safe_vector<T> &list, string path , CMode mode) /* Load/Save All */
{
	filename = path;
	//gprintf("Openning DB: %s\n", filename.c_str());

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

template <typename T>
CCache<T>::CCache(safe_vector<T> &list, string path, T tmp, CMode mode) /* Add One */
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

template <typename T>
CCache<T>::CCache(safe_vector<T> &list, string path, u32 index, CMode mode)  /* Remove One */
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

template <typename T>
CCache<T>::~CCache()
{
	//gprintf("Closing DB: %s\n", filename.c_str());
	if(cache) fclose(cache);
	cache = NULL;
}

template <typename T>
void CCache<T>::SaveAll(safe_vector<T> list)
{
	//gprintf("Updating DB: %s\n", filename.c_str());
	if(!cache) return;
	fwrite((void *)&list[0], 1, list.size() * sizeof(T), cache);
}

template <typename T>
void CCache<T>::SaveOne(T tmp, u32 index)
{
	//gprintf("Updating Item number %u in DB: %s\n", index, filename.c_str());
	if(!cache) return;
	fseek(cache, index * sizeof(T), SEEK_SET);
	fwrite((void *)&tmp, 1, sizeof(T), cache);
}

template <typename T>
void CCache<T>::LoadAll(safe_vector<T> &list)
{
	if(!cache) return;

	//gprintf("Loading DB: %s\n", filename.c_str());

	T tmp;
	fseek(cache, 0, SEEK_END);
	u64 fileSize = ftell(cache);
	fseek(cache, 0, SEEK_SET);

	u32 count = (u32)(fileSize / sizeof(T));
	
	list.reserve(count + list.size());
	for(u32 i = 0; i < count; i++)
	{
		LoadOne(tmp, i);
		list.push_back(tmp);
	}
}


template <typename T>
void CCache<T>::LoadOne(T &tmp, u32 index)
{
	if(!cache) return;

	//gprintf("Fetching Item number %u in DB: %s\n", index, filename.c_str());
	fseek(cache, index * sizeof(T), SEEK_SET);
	fread((void *)&tmp, 1, sizeof(T), cache);
	//gprintf("Path %s\n", tmp.path);
}

template <typename T>
void CCache<T>::AddOne(safe_vector<T> &list, T tmp)
{
	//gprintf("Adding Item number %u in DB: %s\n", list.size()+1, filename.c_str());
	list.push_back(tmp);

	if(!cache) return;
	fwrite((void *)&tmp, 1, sizeof(T), cache);  // FILE* is opened as "ab+" so its always written to the EOF.
}

template <typename T>
void CCache<T>::RemoveOne(safe_vector<T> &list, u32 index)
{
	//gprintf("Removing Item number %u in DB: %s\n", index, filename.c_str());
	list.erase(list.begin() + index);
	SaveAll(list);
}

template class CCache<dir_discHdr>;