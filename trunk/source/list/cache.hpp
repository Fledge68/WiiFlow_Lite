#ifndef CCACHE
#define CCACHE

#include <sys/types.h>
#include <ogcsys.h> 
#include <fstream>
#include "safe_vector.hpp"
#include "disc.h"

//#include "gecko.h"
using namespace std;

const char io[4][5] = {
	"wb",
	"rb",
	"ab",
	"wb",
};

enum CMode
{
	SAVE,
	LOAD,
	ADD,
	REMOVE
};

template <typename T>
class CCache
{
	public:
		 CCache(T &tmp, string path, u32 index, CMode mode);								/* Load/Save One */
		 CCache(safe_vector<T> &list, string path, CMode mode);								/* Load/Save All */
		 CCache(safe_vector<T> &list, string path, T tmp, CMode mode);						/* Add One */
		 CCache(safe_vector<T> &list, string path, u32 index, CMode mode);					/* Remove One */
		~CCache();
	private:
		void SaveAll(safe_vector<T> list);
		void SaveOne(T tmp, u32 index);
		void LoadAll(safe_vector<T> &list);
		void LoadOne(T &tmp, u32 index);
		
		void AddOne(safe_vector<T> &list, T tmp);
		void RemoveOne(safe_vector<T> &list, u32 index);

		FILE *cache;
		string filename;
};
#endif