#ifndef CCACHE
#define CCACHE

#include <sys/types.h>
#include <ogcsys.h> 
#include <fstream>
#include <vector>
#include "loader/disc.h"

//#include "gecko.hpp"
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

class CCache
{
	public:
		 CCache(dir_discHdr &tmp, string path, u32 index, CMode mode);								/* Load/Save One */
		 CCache(vector<dir_discHdr> &list, string path, CMode mode);								/* Load/Save All */
		 CCache(vector<dir_discHdr> &list, string path, dir_discHdr tmp, CMode mode);						/* Add One */
		 CCache(vector<dir_discHdr> &list, string path, u32 index, CMode mode);					/* Remove One */
		~CCache();
	private:
		void SaveAll(vector<dir_discHdr> list);
		void SaveOne(dir_discHdr tmp, u32 index);
		void LoadAll(vector<dir_discHdr> &list);
		void LoadOne(dir_discHdr &tmp, u32 index);
		
		void AddOne(vector<dir_discHdr> &list, dir_discHdr tmp);
		void RemoveOne(vector<dir_discHdr> &list, u32 index);

		FILE *cache;
		string filename;
};
#endif