/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2008-2010
 *
 * networkop.cpp
 *
 * Network and SMB support routines
 ****************************************************************************/

#include <network.h>
#include <malloc.h>
#include <ogc/lwp_watchdog.h>
#include <smb.h>
#include <mxml.h>

#include "snes9xgx.h"
#include "menu.h"
#include "fileop.h"
#include "filebrowser.h"
#include "utils/http.h"
#include "utils/unzip/unzip.h"
#include "utils/unzip/miniunz.h"

static bool networkInit = false;
static bool networkShareInit = false;
char wiiIP[16] = { 0 };

#ifdef HW_RVL
static int netHalt = 0;
static bool updateChecked = true; // true if checked for app update
static char updateURL[128]; // URL of app update
bool updateFound = false; // true if an app update was found

/****************************************************************************
 * UpdateCheck
 * Checks for an update for the application
 ***************************************************************************/

void UpdateCheck()
{
	// we only check for an update if we have internet + SD/USB
	if(updateChecked || !networkInit)
		return;

	if(!isMounted[DEVICE_SD] && !isMounted[DEVICE_USB])
		return;

	updateChecked = true;
	u8 tmpbuffer[256];

	if (http_request("http://snes9x-gx.googlecode.com/svn/trunk/update.xml", NULL, tmpbuffer, 256, SILENT) <= 0)
		return;

	mxml_node_t *xml;
	mxml_node_t *item;

	xml = mxmlLoadString(NULL, (char *)tmpbuffer, MXML_TEXT_CALLBACK);

	if(!xml)
		return;

	// check settings version
	item = mxmlFindElement(xml, xml, "app", "version", NULL, MXML_DESCEND);
	if(item) // a version entry exists
	{
		const char * version = mxmlElementGetAttr(item, "version");

		if(version && strlen(version) == 5)
		{
			int verMajor = version[0] - '0';
			int verMinor = version[2] - '0';
			int verPoint = version[4] - '0';
			int curMajor = APPVERSION[0] - '0';
			int curMinor = APPVERSION[2] - '0';
			int curPoint = APPVERSION[4] - '0';

			// check that the versioning is valid and is a newer version
			if((verMajor >= 0 && verMajor <= 9 &&
				verMinor >= 0 && verMinor <= 9 &&
				verPoint >= 0 && verPoint <= 9) &&
				(verMajor > curMajor ||
				(verMajor == curMajor && verMinor > curMinor) ||
				(verMajor == curMajor && verMinor == curMinor && verPoint > curPoint)))
			{
				item = mxmlFindElement(xml, xml, "file", NULL, NULL, MXML_DESCEND);
				if(item)
				{
					const char * tmp = mxmlElementGetAttr(item, "url");
					if(tmp)
					{
						snprintf(updateURL, 128, "%s", tmp);
						updateFound = false;
					}
				}
			}
		}
	}
	mxmlDelete(xml);
}

static bool unzipArchive(char * zipfilepath, char * unzipfolderpath)
{
	unzFile uf = unzOpen(zipfilepath);
	if (uf==NULL)
		return false;

	if(chdir(unzipfolderpath)) // can't access dir
	{
		makedir(unzipfolderpath); // attempt to make dir
		if(chdir(unzipfolderpath)) // still can't access dir
			return false;
	}

	extractZip(uf,0,1,0);

	unzCloseCurrentFile(uf);
	return true;
}

bool DownloadUpdate()
{
	bool result = false;

	if(updateURL[0] == 0 || appPath[0] == 0 || !ChangeInterface(appPath, NOTSILENT))
	{
		ErrorPrompt("Update failed!");
		updateFound = false; // updating is finished (successful or not!)
		return false;
	}

	// stop checking if devices were removed/inserted
	// since we're saving a file
	HaltDeviceThread();

	int device;
	FindDevice(appPath, &device);

	char updateFile[50];
	sprintf(updateFile, "%s%s Update.zip", pathPrefix[device], APPNAME);

	FILE * hfile = fopen (updateFile, "wb");

	if (hfile)
	{
		if(http_request(updateURL, hfile, NULL, (1024*1024*10), NOTSILENT) > 0)
		{
			fclose (hfile);
			result = unzipArchive(updateFile, (char *)pathPrefix[device]);
		}
		else
		{
			fclose (hfile);
		}
		remove(updateFile); // delete update file
	}

	// go back to checking if devices were inserted/removed
	ResumeDeviceThread();

	if(result)
		InfoPrompt("Update successful!");
	else
		ErrorPrompt("Update failed!");

	updateFound = false; // updating is finished (successful or not!)
	return result;
}

/****************************************************************************
 * InitializeNetwork
 * Initializes the Wii/GameCube network interface
 ***************************************************************************/

static lwp_t networkthread = LWP_THREAD_NULL;
static u8 netstack[32768] ATTRIBUTE_ALIGN (32);

static void * netcb (void *arg)
{
	s32 res=-1;
	int retry;
	int wait;
	static bool prevInit = false;

	while(netHalt != 2)
	{
		retry = 5;
		
		while (retry>0 && (netHalt != 2))
		{			
			if(prevInit) 
			{
				int i;
				net_deinit();
				for(i=0; i < 400 && (netHalt != 2); i++) // 10 seconds to try to reset
				{
					res = net_get_status();
					if(res != -EBUSY) // trying to init net so we can't kill the net
					{
						usleep(2000);
						net_wc24cleanup(); //kill the net 
						prevInit=false; // net_wc24cleanup is called only once
						usleep(20000);
						break;					
					}
					usleep(20000);
				}
			}

			usleep(2000);
			res = net_init_async(NULL, NULL);

			if(res != 0)
			{
				sleep(1);
				retry--;
				continue;
			}

			res = net_get_status();
			wait = 400; // only wait 8 sec
			while (res == -EBUSY && wait > 0  && (netHalt != 2))
			{
				usleep(20000);
				res = net_get_status();
				wait--;
			}

			if(res==0) break;
			retry--;
			usleep(2000);
		}
		if (res == 0)
		{
			struct in_addr hostip;
			hostip.s_addr = net_gethostip();
			if (hostip.s_addr)
			{
				strcpy(wiiIP, inet_ntoa(hostip));
				networkInit = true;	
				prevInit = true;
			}
		}
		if(netHalt != 2) LWP_SuspendThread(networkthread);
	}
	return NULL;
}

/****************************************************************************
 * StartNetworkThread
 *
 * Signals the network thread to resume, or creates a new thread
 ***************************************************************************/
void StartNetworkThread()
{
	netHalt = 0;

	if(networkthread == LWP_THREAD_NULL)
		LWP_CreateThread(&networkthread, netcb, NULL, netstack, 8192, 40);
	else
		LWP_ResumeThread(networkthread);
}

/****************************************************************************
 * StopNetworkThread
 *
 * Signals the network thread to stop
 ***************************************************************************/
void StopNetworkThread()
{
	if(networkthread == LWP_THREAD_NULL || !LWP_ThreadIsSuspended(networkthread))
		return;

	netHalt = 2;
	LWP_ResumeThread(networkthread);

	// wait for thread to finish
	LWP_JoinThread(networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}

#endif

bool InitializeNetwork(bool silent)
{
#ifdef HW_RVL
	StopNetworkThread();

	if(networkInit && net_gethostip() > 0)
		return true;

	networkInit = false;
#else
	if(networkInit)
		return true;
#endif

	int retry = 1;

	while(retry)
	{
		ShowAction("Initializing network...");

#ifdef HW_RVL
		u64 start = gettime();
		StartNetworkThread();

		while (!LWP_ThreadIsSuspended(networkthread))
		{
			usleep(50 * 1000);

			if(diff_sec(start, gettime()) > 10) // wait for 10 seconds max for net init
				break;
		}
#else
		networkInit = !(if_config(wiiIP, NULL, NULL, true) < 0);
#endif

		CancelAction();

		if(networkInit || silent)
			break;

		retry = ErrorPromptRetry("Unable to initialize network!");
		
#ifdef HW_RVL  	
		if(networkInit && net_gethostip() > 0)
#else
		if(networkInit)
#endif
			return true;
	}
	return networkInit;
}

void CloseShare()
{
	if(networkShareInit)
		smbClose("smb");
	networkShareInit = false;
	isMounted[DEVICE_SMB] = false;
}

/****************************************************************************
 * Mount SMB Share
 ****************************************************************************/

bool
ConnectShare (bool silent)
{
	if(!InitializeNetwork(silent))
		return false;

	if(networkShareInit)
		return true;

	int retry = 1;
	int chkS = (strlen(GCSettings.smbshare) > 0) ? 0:1;
	int chkI = (strlen(GCSettings.smbip) > 0) ? 0:1;

	// check that all parameters have been set
	if(chkS + chkI > 0)
	{
		if(!silent)
		{
			char msg[50];
			char msg2[100];
			if(chkS + chkI > 1) // more than one thing is wrong
				sprintf(msg, "Check settings.xml.");
			else if(chkS)
				sprintf(msg, "Share name is blank.");
			else if(chkI)
				sprintf(msg, "Share IP is blank.");

			sprintf(msg2, "Invalid network settings - %s", msg);
			ErrorPrompt(msg2);
		}
		return false;
	}

	while(retry)
	{
		if(!silent)
			ShowAction ("Connecting to network share...");
		
		if(smbInit(GCSettings.smbuser, GCSettings.smbpwd, GCSettings.smbshare, GCSettings.smbip))
			networkShareInit = true;

		if(networkShareInit || silent)
			break;

		retry = ErrorPromptRetry("Failed to connect to network share.");
	}

	if(!silent)
		CancelAction();

	return networkShareInit;
}
