#ifndef CLIST
#define CLIST

#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h>

#include "DeviceHandler.hpp"
#include "safe_vector.hpp"
#include "wbfs_ext.h"
#include "libwbfs/libwbfs.h"
#include "disc.h"
#include "text.hpp"
#include "cache.hpp"

using namespace std;
template <typename T>
class CList
{
    public:
		 CList(){};
		~CList(){};
		void GetPaths(safe_vector<string> &pathlist, string containing, string directory, bool wbfs_fs = false);
		void GetHeaders(safe_vector<string> pathlist, safe_vector<T> &headerlist, string, string);
		void GetChannels(safe_vector<T> &headerlist, string, u32, string);
	private:
		void Check_For_ID(u8 *id, string path, string one, string two);
};
#endif
