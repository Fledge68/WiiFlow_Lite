#ifndef CCACHE
#define CCACHE

#include <sys/types.h>
#include <ogcsys.h> 
#include <fstream>
#include <vector>
#include "loader/disc.h"

//#include "gecko.hpp"
using std::string;
using std::vector;

const char io[2][3] = {
	"wb",
	"rb",
};

enum CMode
{
	SAVE,
	LOAD,
};

class CCache
{
	public:
		 CCache(vector<dir_discHdr> &list, string path, CMode mode);
		~CCache();
	private:
		void SaveAll(vector<dir_discHdr> list);
		void LoadAll(vector<dir_discHdr> &list);

		FILE *cache;
		string filename;
};
#endif