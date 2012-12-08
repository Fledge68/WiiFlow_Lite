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
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include "GameTDB.hpp"
#include "video.hpp"
#include "defines.h"
#include "text.hpp"
#include "config/config.hpp"
#include "gecko/gecko.hpp"
#include "memory/mem2.hpp"
#define NAME_OFFSET_DB	"gametdb_offsets.bin"
#define MAXREADSIZE		1024*1024   //Cache size only for parsing the offsets: 1MB

typedef struct _ReplaceStruct
{
	const char *orig;
	char replace;
	short size;
} ReplaceStruct;

//! More replacements can be added if needed
static const ReplaceStruct Replacements[] =
{
	{ "&gt;", '>', 4 },
	{ "&lt;", '<', 4 },
	{ "&quot;", '\"', 6 },
	{ "&apos;", '\'', 6 },
	{ "&amp;", '&', 5 },
	{ NULL, '\0', 0 }
};

GameTDB::GameTDB()
	: isLoaded(false), isParsed(false), file(0), filepath(0), LangCode("EN"), GameNodeCache(NULL)
{
}

GameTDB::GameTDB(const char *filepath)
	: isLoaded(false), isParsed(false), file(0), filepath(0), LangCode("EN"), GameNodeCache(NULL)
{
	OpenFile(filepath);
}

GameTDB::~GameTDB()
{
	CloseFile();
}

bool GameTDB::OpenFile(const char *filepath)
{
	if(!filepath)
		return false;

	//gprintf("Trying to open '%s'...", filepath);
	file = fopen(filepath, "rb");
	if(file)
	{
		this->filepath = filepath;

		//gprintf("success\n");

		int pos;
		string OffsetsPath = filepath;
		if((pos = OffsetsPath.find_last_of('/')) != (int) string::npos)
			OffsetsPath[pos] = '\0';
		else
			OffsetsPath.clear(); //! Relative path

		//gprintf("Checking game offsets\n");
		LoadGameOffsets(OffsetsPath.c_str());
		/*if (!isParsed)
		{
		gprintf("Checking titles.ini\n");
		CheckTitlesIni(OffsetsPath.c_str());
		}*/
	}
	//else gprintf("failed\n");

	isLoaded = (file != NULL);
	return isLoaded;
}

void GameTDB::CloseFile()
{
	OffsetMap.clear();

	if(GameNodeCache)
		MEM2_free(GameNodeCache);
	GameNodeCache = NULL;

	if(file)
		fclose(file);
	file = NULL;
}

void GameTDB::Refresh()
{
	gprintf("Refreshing file '%s'\n", filepath);
	CloseFile();

	if(filepath == NULL)
		return;

	OpenFile(filepath);
}

bool GameTDB::LoadGameOffsets(const char *path)
{
	if(!path)
		return false;

	string OffsetDBPath = path;
	if(strlen(path) > 0 && path[strlen(path)-1] != '/')
		OffsetDBPath += '/';
	OffsetDBPath += NAME_OFFSET_DB;

	FILE *fp = fopen(OffsetDBPath.c_str(), "rb");
	if(!fp)
	{
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());
		return result;
	}

	u64 ExistingVersion = GetGameTDBVersion();
	u64 Version = 0;
	u32 NodeCount = 0;

	fread(&Version, 1, sizeof(Version), fp);

	if(ExistingVersion != Version)
	{
		fclose(fp);
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());
		return result;
	}

	fread(&NodeCount, 1, sizeof(NodeCount), fp);

	if(NodeCount == 0)
	{
		fclose(fp);
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());
		return result;
	}

	OffsetMap.resize(NodeCount);

	if(fread(&OffsetMap[0], 1, NodeCount*sizeof(GameOffsets), fp) != NodeCount*sizeof(GameOffsets))
	{
		fclose(fp);
		bool result = ParseFile();
		if(result)
			SaveGameOffsets(OffsetDBPath.c_str());
		return result;
	}

	fclose(fp);

	return true;
}

bool GameTDB::SaveGameOffsets(const char *path)
{
	if(OffsetMap.size() == 0 || !path)
	return false;

	FILE *fp = fopen(path, "wb");
	if(!fp)
		return false;

	u64 ExistingVersion = GetGameTDBVersion();
	u32 NodeCount = OffsetMap.size();

	if(fwrite(&ExistingVersion, 1, sizeof(ExistingVersion), fp) != sizeof(ExistingVersion))
	{
		fclose(fp);
		return false;
	}

	if(fwrite(&NodeCount, 1, sizeof(NodeCount), fp) != sizeof(NodeCount))
	{
		fclose(fp);
		return false;
	}

	if(fwrite(&OffsetMap[0], 1, NodeCount*sizeof(GameOffsets), fp) != NodeCount*sizeof(GameOffsets))
	{
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}

u64 GameTDB::GetGameTDBVersion()
{
	if(!file)
		return 0;

	char TmpText[1024];

	if(GetData(TmpText, 0, sizeof(TmpText)) < 0)
		return 0;

	char *VersionText = GetNodeText(TmpText, "<GameTDB version=\"", "/>");
	if(!VersionText)
		return 0;

	return strtoull(VersionText, NULL, 10);
}

int GameTDB::GetData(char *data, int offset, int size)
{
	if(!file || !data)
		return -1;

	fseek(file, offset, SEEK_SET);

	return fread(data, 1, size, file);
}

char *GameTDB::LoadGameNode(const char *id)
{
	u32 read = 0;

	GameOffsets *offset = this->GetGameOffset(id);
	if(!offset)
		return NULL;

	char *data = (char*)MEM2_alloc(offset->nodesize+1);
	if(!data)
		return NULL;

	if((read = GetData(data, offset->gamenode, offset->nodesize)) != offset->nodesize)
	{
		MEM2_free(data);
		return NULL;
	}

	data[read] = '\0';

	return data;
}

char *GameTDB::GetGameNode(const char *id)
{
	char *data = NULL;

	if(GameNodeCache != NULL && strncmp(id, GameIDCache, strlen(GameIDCache)) == 0)
	{
		data = (char*)MEM2_alloc(strlen(GameNodeCache)+1);
		if(data)
			strcpy(data, GameNodeCache);
	}
	else
	{
		if(GameNodeCache)
			MEM2_free(GameNodeCache);
		GameNodeCache = LoadGameNode(id);
		if(GameNodeCache)
		{
			snprintf(GameIDCache, sizeof(GameIDCache), id);
			data = (char*)MEM2_alloc(strlen(GameNodeCache)+1);
			if(data)
				strcpy(data, GameNodeCache);
		}
	}

	return data;
}

GameOffsets *GameTDB::GetGameOffset(const char *gameID)
{
	for(u32 i = 0; i < OffsetMap.size(); ++i)
	{
		if(strncmp(gameID, OffsetMap[i].gameID, strlen(OffsetMap[i].gameID)) == 0)
			return &OffsetMap[i];
	}

	return 0;
}

static inline char *CleanText(char * &in_text)
{
	if(!in_text)
		return NULL;

	const char *ptr = in_text;
	char *text = in_text;

	while(*ptr != '\0')
	{
		for(int i = 0; Replacements[i].orig != 0; ++i)
		{
			if(strncmp(ptr, Replacements[i].orig, Replacements[i].size) == 0)
			{
				ptr += Replacements[i].size;
				*text = Replacements[i].replace;
				++text;
				i = 0;
				continue;
			}
		}

		if(*ptr == '\r')
		{
			++ptr;
			continue;
		}

		*text = *ptr;
		++ptr;
		++text;
	}

	*text = '\0';

	return in_text;
}

char *GameTDB::GetNodeText(char *data, const char *nodestart, const char *nodeend)
{
	if(!data || !nodestart || !nodeend)
		return NULL;

	char *position = strstr(data, nodestart);
	if(!position)
		return NULL;

	position += strlen(nodestart);

	char *end = strstr(position, nodeend);
	if(!end)
		return NULL;

	*end = '\0';

	return CleanText(position);
}

char *GameTDB::SeekLang(char *text, const char *langcode)
{
	if(!text || !langcode)
		return NULL;

	char *ptr = text;
	while((ptr = strstr(ptr, "<locale lang=")) != NULL)
	{
		ptr += strlen("<locale lang=\"");

		if(strncmp(ptr, langcode, strlen(langcode)) == 0)
		{
			//! Cut off all the other languages
			char *end = strstr(ptr, "</locale>");
			if(!end)
				return NULL;

			end += strlen("</locale>");
			*end = '\0';

			return ptr;
		}
	}

	return NULL;
}

bool GameTDB::ParseFile()
{
	OffsetMap.clear();

	if(!file)
		return false;

	char *Line = (char*)MEM2_alloc(MAXREADSIZE+1);
	if(!Line)
		return false;

	bool readnew = false;
	int i, currentPos = 0;
	int read = 0;
	const char *gameNode = NULL;
	const char *idNode = NULL;
	const char *gameEndNode = NULL;

	while((read = GetData(Line, currentPos, MAXREADSIZE)) > 0)
	{
		gameNode = Line;
		readnew = false;

		//! Ensure the null termination at the end
		Line[read] = '\0';

		while((gameNode = strstr(gameNode, "<game name=\"")) != NULL)
		{
			idNode = strstr(gameNode, "<id>");
			gameEndNode = strstr(gameNode, "</game>");
			if(!idNode || !gameEndNode)
			{
				//! We are in the middle of the game node, reread complete node and more
				currentPos += (gameNode-Line);
				fseek(file, currentPos, SEEK_SET);
				readnew = true;
				break;
			}

			idNode += strlen("<id>");
			gameEndNode += strlen("</game>");

			int size = OffsetMap.size();
			OffsetMap.resize(size+1);

			for(i = 0; i < 7 && *idNode != '<'; ++i, ++idNode)
				OffsetMap[size].gameID[i] = *idNode;
			OffsetMap[size].gameID[i] = '\0';
			OffsetMap[size].gamenode = currentPos+(gameNode-Line);
			OffsetMap[size].nodesize = (gameEndNode-gameNode);
			gameNode = gameEndNode;
		}
		if(readnew)
			continue;
		currentPos += read;
	}
	MEM2_free(Line);
	return true;
}

bool GameTDB::FindTitle(char *data, const char * &title, const string &langCode)
{
	char *language = SeekLang(data, langCode.c_str());
	if(language == NULL)
	{
		language = SeekLang(data, "EN");
		if(language == NULL)
			return false;
	}
	title = GetNodeText(language, "<title>", "</title>");

	if(title == NULL)
		return false;
	return true;
}

bool GameTDB::GetTitle(const char *id, const char * &title)
{
	title = NULL;
	if(id == NULL)
		return false;

	char *data = GetGameNode(id);
	if(data == NULL)
		return false;
	bool retval = FindTitle(data, title, LangCode);
	MEM2_free(data);

	return retval;
}

bool GameTDB::GetSynopsis(const char *id, const char * &synopsis)
{
	synopsis = NULL;
	if(!id)
		return false;

	char *data = GetGameNode(id);
	if(!data)
		return false;

	char *language = SeekLang(data, LangCode.c_str());
	if(language == NULL)
	{
		language = SeekLang(data, "EN");
		if(language == NULL)
		{
			MEM2_free(data);
			return false;
		}
	}
	synopsis = GetNodeText(language, "<synopsis>", "</synopsis>");
	MEM2_free(data);

	if(synopsis == NULL)
		return false;
	return true;
}

bool GameTDB::GetRegion(const char *id, const char * &region)
{
	region = NULL;
	if(!id)
		return false;

	char *data = GetGameNode(id);
	if(!data)
		return false;
	region = GetNodeText(data, "<region>", "</region>");
	MEM2_free(data);

	if(region == NULL)
		return false;
	return true;
}

bool GameTDB::GetDeveloper(const char *id, const char * &dev)
{
	dev = NULL;
	if(id == NULL)
		return false;

	char *data = GetGameNode(id);
	if(data == NULL)
		return false;

	dev = GetNodeText(data, "<developer>", "</developer>");
	MEM2_free(data);

	if(dev == NULL)
		return false;
	return true;
}

bool GameTDB::GetPublisher(const char *id, const char * &pub)
{
	pub = NULL;
	if(!id)
		return false;

	char *data = GetGameNode(id);
	if(data == NULL)
		return false;

	pub = GetNodeText(data, "<publisher>", "</publisher>");
	MEM2_free(data);

	if(pub == NULL)
		return false;
	return true;
}

u32 GameTDB::GetPublishDate(const char *id)
{
	if(!id)
		return 0;

	char *data = GetGameNode(id);
	if(!data)
		return 0;

	char *year_string = GetNodeText(data, "<date year=\"", "/>");
	if(!year_string)
	{
		MEM2_free(data);
		return 0;
	}

	u32 year, day, month;

	year = atoi(year_string);

	char *month_string = strstr(year_string, "month=\"");
	if(!month_string)
	{
		MEM2_free(data);
		return 0;
	}

	month_string += strlen("month=\"");

	month = atoi(month_string);

	char *day_string = strstr(month_string, "day=\"");
	if(!day_string)
	{
		MEM2_free(data);
		return 0;
	}

	day_string += strlen("day=\"");

	day = atoi(day_string);

	MEM2_free(data);

	return ((year & 0xFFFF) << 16 | (month & 0xFF) << 8 | (day & 0xFF));
}

bool GameTDB::GetGenres(const char *id, const char * &gen)
{
	gen = NULL;

	if(id == NULL)
		return false;

	char *data = GetGameNode(id);
	if(data == NULL)
		return false;

	gen = GetNodeText(data, "<genre>", "</genre>");
	MEM2_free(data);

	if(gen == NULL)
		return false;
	return true;
}

const char *GameTDB::RatingToString(int rating)
{
	switch(rating)
	{
		case GAMETDB_RATING_TYPE_CERO:
			return "CERO";
		case GAMETDB_RATING_TYPE_ESRB:
			return "ESRB";
		case GAMETDB_RATING_TYPE_PEGI:
			return "PEGI";
		case GAMETDB_RATING_TYPE_GRB:
			return "GRB";
		default:
			break;
	}

	return NULL;
}

int GameTDB::GetRating(const char *id)
{
	int rating = -1;

	if(!id)
		return rating;

	char *data = GetGameNode(id);
	if(!data)
		return rating;

	char *rating_text = GetNodeText(data, "<rating type=\"", "/>");
	if(!rating_text)
	{
		MEM2_free(data);
		return rating;
	}

	if(strncmp(rating_text, "CERO", 4) == 0)
		rating = GAMETDB_RATING_TYPE_CERO;
	else if(strncmp(rating_text, "ESRB", 4) == 0)
		rating = GAMETDB_RATING_TYPE_ESRB;
	else if(strncmp(rating_text, "PEGI", 4) == 0)
		rating = GAMETDB_RATING_TYPE_PEGI;
	else if(strncmp(rating_text, "GRB", 4) == 0)
		rating = GAMETDB_RATING_TYPE_GRB;

	MEM2_free(data);

	return rating;
}

bool GameTDB::GetRatingValue(const char *id, const char * &rating_value)
{
	rating_value = NULL;
	if(!id)
		return false;

	char *data = GetGameNode(id);
	if(!data)
		return false;

	char *rating_text = GetNodeText(data, "<rating type=\"", "/>");
	if(!rating_text)
	{
		MEM2_free(data);
		return false;
	}

	rating_value = GetNodeText(rating_text, "value=\"", "\"");
	MEM2_free(data);

	if(rating_value == NULL)
		return false;
	return true;
}

int GameTDB::GetRatingDescriptors(const char *id, vector<string> & desc_list)
{
	desc_list.clear();
	if(!id)
		return -1;

	char *data = GetGameNode(id);
	if(!data)
		return -1;

	char *descriptor_text = GetNodeText(data, "<descriptor>", "</rating>");
	if(!descriptor_text)
	{
		MEM2_free(data);
		return -1;
	}

	u32 list_num = 0;

	while(*descriptor_text != '\0')
	{
		if(strncmp(descriptor_text, "</descriptor>", strlen("</descriptor>")) == 0)
		{
			desc_list[list_num].push_back('\0');
			descriptor_text = strstr(descriptor_text, "<descriptor>");
			if(!descriptor_text)
				break;

			descriptor_text += strlen("<descriptor>");
			list_num++;
		}

		if(list_num >= desc_list.size())
			desc_list.resize(list_num+1);

		desc_list[list_num].push_back(*descriptor_text);
		++descriptor_text;
	}

	MEM2_free(data);

	return desc_list.size();
}

int GameTDB::GetWifiPlayers(const char *id)
{
	int players = -1;

	if(!id)
		return players;

	char *data = GetGameNode(id);
	if(!data)
		return players;

	char *PlayersNode = GetNodeText(data, "<wi-fi players=\"", "\">");
	if(!PlayersNode)
	{
		MEM2_free(data);
		return players;
	}

	players = atoi(PlayersNode);

	return players;
}

int GameTDB::GetWifiFeatures(const char *id, vector<string> & feat_list)
{
	feat_list.clear();
	if(!id)
		return -1;

	char *data = GetGameNode(id);
	if(!data)
		return -1;

	char *feature_text = GetNodeText(data, "<feature>", "</wi-fi>");
	if(!feature_text)
	{
		MEM2_free(data);
		return -1;
	}

	u32 list_num = 0;

	while(*feature_text != '\0')
	{
		if(strncmp(feature_text, "</feature>", strlen("</feature>")) == 0)
		{
			feat_list[list_num].push_back('\0');
			feature_text = strstr(feature_text, "<feature>");
			if(!feature_text)
				break;

			feature_text += strlen("<feature>");
			list_num++;
		}

		if(list_num >= feat_list.size())
			feat_list.resize(list_num+1);

		if(feat_list[list_num].size() == 0)
			feat_list[list_num].push_back(toupper((int)*feature_text));
		else
			feat_list[list_num].push_back(*feature_text);

		++feature_text;
	}

	MEM2_free(data);

	return feat_list.size();
}

int GameTDB::GetPlayers(const char *id)
{
	int players = -1;

	if(!id)
		return players;

	char *data = GetGameNode(id);
	if(!data)
		return players;

	char *PlayersNode = GetNodeText(data, "<input players=\"", "\">");
	if(!PlayersNode)
	{
		MEM2_free(data);
		return players;
	}

	players = atoi(PlayersNode);

	return players;
}

int GameTDB::GetAccessories(const char *id, vector<Accessory> & acc_list)
{
	acc_list.clear();
	if(!id)
		return -1;

	char *data = GetGameNode(id);
	if(!data)
		return -1;

	char *ControlsNode = GetNodeText(data, "<control type=\"", "</input>");
	if(!ControlsNode)
	{
		MEM2_free(data);
		return -1;
	}

	u32 list_num = 0;

	while(ControlsNode && *ControlsNode != '\0')
	{
		if(list_num >= acc_list.size())
			acc_list.resize(list_num+1);

		for(const char *ptr = ControlsNode; *ptr != '"' && *ptr != '\0'; ptr++)
			acc_list[list_num].Name.push_back(*ptr);
		acc_list[list_num].Name.push_back('\0');

		char *requiredField = strstr(ControlsNode, "required=\"");
		if(!requiredField)
		{
			MEM2_free(data);
			return -1;
		}

		requiredField += strlen("required=\"");

		acc_list[list_num].Required = strncmp(requiredField, "true", 4) == 0;

		ControlsNode = strstr(requiredField, "<control type=\"");
		if(ControlsNode)
			ControlsNode += strlen("<control type=\"");

		list_num++;
	}

	MEM2_free(data);

	return acc_list.size();
}

u32 GameTDB::FindCaseColor(char *data)
{
	u32 color = -1;

	char *ColorNode = GetNodeText(data, "<case color=\"", "\"");
	if(!ColorNode || strlen(ColorNode) == 0)
		return color;

	char format[8];
	sprintf(format, "0x%s", ColorNode);

	return strtoul(format, NULL, 16);
}

u32 GameTDB::GetCaseColor(const char *id)
{
	u32 color = -1;
	if(!id)
		return color;

	char *data = GetGameNode(id);
	if(!data)
		return color;

	color = FindCaseColor(data);

	if(color != 0xffffffff)
		gprintf("GameTDB: Found alternate color(%x) for: %s\n", color, id);

	MEM2_free(data);
	return color;
}

int GameTDB::GetCaseVersions(const char *id)
{
	int altcase = -1;

	if(!id)
		return altcase;

	char *data = GetGameNode(id);
	if(!data)
	{
		//gprintf("GameTDB: GameNode for %s not found\n", id);
		return altcase;
	}

	char *PlayersNode = GetNodeText(data, "case versions=\"", "\"");
	if(!PlayersNode)
	{
		MEM2_free(data);
		return altcase;
	}

	altcase = atoi(PlayersNode);

	return altcase;
}

bool GameTDB::IsLoaded()
{
	return isLoaded;
}
