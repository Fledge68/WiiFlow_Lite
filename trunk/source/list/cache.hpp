#ifndef CCACHE
#define CCACHE

#include <sys/types.h>
#include <ogcsys.h> 
#include <fstream>
#include <vector>
#include "loader/disc.h"

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
		 CCache(vector<T> &list, string path, CMode mode);								/* Load/Save All */
		 CCache(vector<T> &list, string path, T tmp, CMode mode);						/* Add One */
		 CCache(vector<T> &list, string path, u32 index, CMode mode);					/* Remove One */
		~CCache();
	private:
		void SaveAll(vector<T> list);
		void SaveOne(T tmp, u32 index);
		void LoadAll(vector<T> &list);
		void LoadOne(T &tmp, u32 index);
		
		void AddOne(vector<T> &list, T tmp);
		void RemoveOne(vector<T> &list, u32 index);

		FILE *cache;
		string filename;
};
#endif