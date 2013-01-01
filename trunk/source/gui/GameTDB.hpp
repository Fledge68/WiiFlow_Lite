/****************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#ifndef GAMETDB_HPP_
#define GAMETDB_HPP_

#include <vector>
#include <string>
#include <gccore.h>

using namespace std;

enum
{
	GAMETDB_RATING_TYPE_CERO,
	GAMETDB_RATING_TYPE_ESRB,
	GAMETDB_RATING_TYPE_PEGI,
	GAMETDB_RATING_TYPE_GRB
};

typedef struct _Accessory
{
	string Name;
	bool Required;
} Accessory;

typedef struct _GameOffsets
{
	char gameID[7];
	unsigned int gamenode;
	unsigned int nodesize;
} ATTRIBUTE_PACKED GameOffsets;

class GameTDB
{
public:
	//! Constructor
	GameTDB();
	//! Constructor
	//! If filepath is passed the xml file is opened and the node offsets are loaded
	GameTDB(const char * filepath);
	//! Destructor
	~GameTDB();
	//! If filepath is passed the xml file is opened and the node offsets are loaded
	bool OpenFile(const char * filepath);
	//! Closes the GameTDB xml file
	void CloseFile();
	//! Refresh the GameTDB xml file, in case the file has been updated
	void Refresh();
	//! Set the language code which should be use to find the appropriate language
	//! If the language code is not found, the language code defaults to EN
	void SetLanguageCode(const char * code) { if(code) LangCode = code; };
	//! Get the current set language code
	const char * GetLanguageCode() { return LangCode.c_str(); };
	//! Get the title of a specific game id in the language defined in LangCode
	bool GetTitle(const char *id, const char * &title);
	//! Get the synopsis of a specific game id in the language defined in LangCode
	bool GetSynopsis(const char *id, const char * &synopsis);
	//! Get the region of a game for a specific game id
	bool GetRegion(const char *id, const char * &region);
	//! Get the developer of a game for a specific game id
	bool GetDeveloper(const char *id, const char * &dev);
	//! Get the publisher of a game for a specific game id
	bool GetPublisher(const char *id, const char * &pub);
	//! Get the publish date of a game for a specific game id
	//! First 1 byte is the day, than 1 byte month and last 2 bytes is the year
	//! year = (return >> 16), month = (return >> 8) & 0xFF, day = return & 0xFF
	unsigned int GetPublishDate(const char *id);
	//! Get the genre list of a game for a specific game id
	bool GetGenres(const char * id, const char * &gen);
	//! Get the rating type for a specific game id
	//! The rating type can be converted to a string with GameTDB::RatingToString(rating)
	int GetRating(const char * id);
	//! Get the rating value for a specific game id
	bool GetRatingValue(const char * id, const char * &rating_value);
	//! Get the rating descriptor list inside a vector for a specific game id
	//! Returns the amount of descriptors found or -1 if failed
	int GetRatingDescriptors(const char * id, vector<string> & desc_list);
	//! Get the wifi player count for a specific game id
	//! Returns the amount of wifi players or -1 if failed
	int GetWifiPlayers(const char * id);
	//! Get the wifi feature list inside a vector for a specific game id
	//! Returns the amount of wifi features found or -1 if failed
	int GetWifiFeatures(const char * id, vector<string> & feat_list);
	//! Get the player count for a specific game id
	//! Returns the amount of players or -1 if failed
	int GetPlayers(const char * id);
	//! Returns the amount of accessoires found or -1 if failed
	//! Get the accessoire (inputs) list inside a vector for a specific game id
	int GetAccessories(const char * id, vector<Accessory> & acc_list);
	//! Get the box (case) color for a specific game id
	//! Returns the color in RGB (first 3 bytes)
	unsigned int GetCaseColor(const char * id);
	int GetCaseVersions(const char * id);
	//! Convert a specific game rating to a string
	static const char * RatingToString(int rating);
	//! Get the version of the gametdb xml database
	unsigned long long GetGameTDBVersion();
	//! Get the entry count in the xml database
	inline size_t GetEntryCount() { return OffsetMap.size(); };
	//! Is a database loaded
	bool IsLoaded();
private:
	bool ParseFile();
	bool LoadGameOffsets(const char * path);
	bool SaveGameOffsets(const char * path);
	bool CheckTitlesIni(const char * path);
	bool FindTitle(char *data, const char * &title, const string &langCode);
	unsigned int FindCaseColor(char * data);
	inline int GetData(char * data, int offset, int size);
	inline char * LoadGameNode(const char * id);
	inline char * GetGameNode(const char * id);
	inline GameOffsets * GetGameOffset(const char * id);
	inline char * SeekLang(char * text, const char * langcode);
	inline char * GetNodeText(char *data, const char *nodestart, const char *nodeend);

	bool isLoaded;
	bool isParsed;
	vector<GameOffsets> OffsetMap;
	FILE * file;
	const char *filepath;
	string LangCode;
	char *GameNodeCache;
	char GameIDCache[7];
};

#endif
