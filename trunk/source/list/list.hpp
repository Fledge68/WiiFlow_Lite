#ifndef CLIST
#define CLIST

#include <ogcsys.h>
#include <dirent.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h>

#include "cache.hpp"
#include "config/config.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gui/text.hpp"
#include "loader/disc.h"
#include "loader/wbfs_ext.h"
#include "libwbfs/libwbfs.h"

template <typename T>
class CList
{
    public:
		 CList(){};
		~CList(){};
		void GetPaths(vector<string> &pathlist, string containing, string directory, bool wbfs_fs = false, bool dml = false, bool depth_limit = true);
		void GetHeaders(vector<string> pathlist, vector<T> &headerlist, string, string, string, Config &plugin);
		void GetChannels(vector<T> &headerlist, string, u32, string);
	private:
		void Check_For_ID(char *id, string path, string one, string two);
};
#endif
