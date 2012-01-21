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
#include "config.hpp"
#include "video.hpp"
#include "gecko.h"
#include "defines.h"

#define NAME_OFFSET_DB  "gametdb_offsets.bin"
#define MAXREADSIZE     1024*1024   // Cache size only for parsing the offsets: 1MB

typedef struct _ReplaceStruct
{
    const char * orig;
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
	: isLoaded(false), isParsed(false), file(0), filepath(0), LangCode("EN"), GameNodeCache(0)
{
}

GameTDB::GameTDB(const char * filepath)
	: isLoaded(false), isParsed(false), file(0), filepath(0), LangCode("EN"), GameNodeCache(0)
{
    OpenFile(filepath);
}

GameTDB::~GameTDB()
{
    CloseFile();
}

bool GameTDB::OpenFile(const char * filepath)
{
    if(!filepath) return false;

	gprintf("Trying to open '%s'...", filepath);
    file = fopen(filepath, "rb");
    if(file)
    {
		this->filepath = filepath;
		
		gprintf("success\n");

        int pos;
        string OffsetsPath = filepath;
        if((pos = OffsetsPath.find_last_of('/')) != (int) string::npos)
            OffsetsPath[pos] = '\0';
        else
            OffsetsPath.clear(); //! Relative path

		gprintf("Checking game offsets\n");
        LoadGameOffsets(OffsetsPath.c_str());
		/*if (!isParsed)
		{
			gprintf("Checking titles.ini\n");
			CheckTitlesIni(OffsetsPath.c_str());
		}*/
    }
	else gprintf("failed\n");

    isLoaded = (file != NULL);
	return isLoaded;
}

void GameTDB::CloseFile()
{
    OffsetMap.clear();

    if(GameNodeCache)
        delete [] GameNodeCache;
    GameNodeCache = NULL;

    if(file) fclose(file);
    file = NULL;
}

void GameTDB::Refresh()
{
	gprintf("Refreshing file '%s'\n", filepath);
	CloseFile();
	
	if (filepath == NULL)
		return;
		
	OpenFile(filepath);
}

bool GameTDB::LoadGameOffsets(const char * path)
{
    if(!path) return false;

    string OffsetDBPath = path;
    if(strlen(path) > 0 && path[strlen(path)-1] != '/')
        OffsetDBPath += '/';
    OffsetDBPath += NAME_OFFSET_DB;

    FILE * fp = fopen(OffsetDBPath.c_str(), "rb");
    if(!fp)
    {
        bool result = ParseFile();
        if(result)
            SaveGameOffsets(OffsetDBPath.c_str());

        return result;
    }

    unsigned long long ExistingVersion = GetGameTDBVersion();
    unsigned long long Version = 0;
    unsigned int NodeCount = 0;

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

bool GameTDB::SaveGameOffsets(const char * path)
{
    if(OffsetMap.size() == 0 || !path)
        return false;

    FILE * fp = fopen(path, "wb");
    if(!fp) return false;

    unsigned long long ExistingVersion = GetGameTDBVersion();
    unsigned int NodeCount = OffsetMap.size();

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

unsigned long long GameTDB::GetGameTDBVersion()
{
    if(!file)
        return 0;

    char TmpText[1024];

    if(GetData(TmpText, 0, sizeof(TmpText)) < 0)
        return 0;

    char * VersionText = GetNodeText(TmpText, "<GameTDB version=\"", "/>");
    if(!VersionText)
        return 0;

    return strtoull(VersionText, NULL, 10);
}

int GameTDB::GetData(char * data, int offset, int size)
{
    if(!file || !data)
        return -1;

    fseek(file, offset, SEEK_SET);

    return fread(data, 1, size, file);
}

char * GameTDB::LoadGameNode(const char * id)
{
    unsigned int read = 0;

    GameOffsets * offset = this->GetGameOffset(id);
    if(!offset)
        return NULL;

    char * data = new (std::nothrow) char[offset->nodesize+1];
    if(!data)
        return NULL;

    if((read = GetData(data, offset->gamenode, offset->nodesize)) != offset->nodesize)
    {
        delete [] data;
        return NULL;
    }

    data[read] = '\0';

    return data;
}

char * GameTDB::GetGameNode(const char * id)
{
    char * data = NULL;

    if(GameNodeCache != 0 && strncmp(id, GameIDCache, strlen(GameIDCache)) == 0)
    {
        data = new (std::nothrow) char[strlen(GameNodeCache)+1];
        if(data)
            strcpy(data, GameNodeCache);
    }
    else
    {
        if(GameNodeCache)
            delete [] GameNodeCache;

        GameNodeCache = LoadGameNode(id);

        if(GameNodeCache)
        {
            snprintf(GameIDCache, sizeof(GameIDCache), id);
            data = new (std::nothrow) char[strlen(GameNodeCache)+1];
            if(data)
                strcpy(data, GameNodeCache);
        }
    }

    return data;
}

GameOffsets * GameTDB::GetGameOffset(const char * gameID)
{
    for(unsigned int i = 0; i < OffsetMap.size(); ++i)
    {
        if(strncmp(gameID, OffsetMap[i].gameID, strlen(OffsetMap[i].gameID)) == 0)
            return &OffsetMap[i];
    }

    return 0;
}

static inline char * CleanText(char * in_text)
{
    if(!in_text)
        return NULL;

    const char * ptr = in_text;
    char * text = in_text;

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

char * GameTDB::GetNodeText(char * data, const char * nodestart, const char * nodeend)
{
    if(!data || !nodestart || !nodeend)
        return NULL;

    char * position = strstr(data, nodestart);
    if(!position)
        return NULL;

    position += strlen(nodestart);

    char * end = strstr(position, nodeend);
    if(!end)
        return NULL;

    *end = '\0';

    return CleanText(position);
}

char * GameTDB::SeekLang(char * text, const char * langcode)
{
    if(!text || !langcode) return NULL;

    char * ptr = text;
    while((ptr = strstr(ptr, "<locale lang=")) != NULL)
    {
        ptr += strlen("<locale lang=\"");

        if(strncmp(ptr, langcode, strlen(langcode)) == 0)
        {
            //! Cut off all the other languages
            char * end = strstr(ptr, "</locale>");
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

    char * Line = new (std::nothrow) char[MAXREADSIZE+1];
    if(!Line)
        return false;

    bool readnew = false;
    int i, currentPos = 0;
    int read = 0;
    const char * gameNode = NULL;
    const char * idNode = NULL;
    const char * gameEndNode = NULL;

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

    delete [] Line;

    return true;
}

bool GameTDB::FindTitle(char * data, string & title, string langCode)
{
    char * language = SeekLang(data, langCode.c_str());
    if(!language)
    {
        language = SeekLang(data, "EN");
        if(!language)
        {
            return false;
        }
    }

    char * the_title = GetNodeText(language, "<title>", "</title>");
    if(!the_title)
    {
        return false;
    }

    title = the_title;
	return true;
}

bool GameTDB::GetTitle(const char * id, string & title)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

	bool retval = FindTitle(data, title, LangCode);

    delete [] data;

    return retval;
}

bool GameTDB::GetSynopsis(const char * id, string & synopsis)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

    char * language = SeekLang(data, LangCode.c_str());
    if(!language)
    {
        language = SeekLang(data, "EN");
        if(!language)
        {
            delete [] data;
            return false;
        }
    }

    char * the_synopsis = GetNodeText(language, "<synopsis>", "</synopsis>");
    if(!the_synopsis)
    {
        delete [] data;
        return false;
    }

    synopsis = the_synopsis;

    delete [] data;

    return true;
}

bool GameTDB::GetRegion(const char * id, string & region)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

    char * the_region = GetNodeText(data, "<region>", "</region>");
    if(!the_region)
    {
        delete [] data;
        return false;
    }

    region = the_region;

    delete [] data;

    return true;
}

bool GameTDB::GetDeveloper(const char * id, string & dev)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

    char * the_dev = GetNodeText(data, "<developer>", "</developer>");
    if(!the_dev)
    {
        delete [] data;
        return false;
    }

    dev = the_dev;

    delete [] data;

    return true;
}

bool GameTDB::GetPublisher(const char * id, string & pub)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

    char * the_pub = GetNodeText(data, "<publisher>", "</publisher>");
    if(!the_pub)
    {
        delete [] data;
        return false;
    }

    pub = the_pub;

    delete [] data;

    return true;
}

unsigned int GameTDB::GetPublishDate(const char * id)
{
    if(!id) return 0;

    char * data = GetGameNode(id);
    if(!data) return 0;

    char * year_string = GetNodeText(data, "<date year=\"", "/>");
    if(!year_string)
    {
        delete [] data;
        return 0;
    }

    unsigned int year, day, month;

    year = atoi(year_string);

    char * month_string = strstr(year_string, "month=\"");
    if(!month_string)
    {
        delete [] data;
        return 0;
    }

    month_string += strlen("month=\"");

    month = atoi(month_string);

    char * day_string = strstr(month_string, "day=\"");
    if(!day_string)
    {
        delete [] data;
        return 0;
    }

    day_string += strlen("day=\"");

    day = atoi(day_string);

    delete [] data;

    return ((year & 0xFFFF) << 16 | (month & 0xFF) << 8 | (day & 0xFF));
}

bool GameTDB::GetGenres(const char * id, safe_vector<string> & genre)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

    char * the_genre = GetNodeText(data, "<genre>", "</genre>");
    if(!the_genre)
    {
        delete [] data;
        return false;
    }

    unsigned int genre_num = 0;
    const char * ptr = the_genre;

    while(*ptr != '\0')
    {
        if(genre_num >= genre.size())
            genre.resize(genre_num+1);

        if(*ptr == ',' || *ptr == '/' || *ptr == ';')
        {
            ptr++;
            while(*ptr == ' ') ptr++;
            genre[genre_num].push_back('\0');
            genre_num++;
            continue;
        }

        if(genre[genre_num].size() == 0)
            genre[genre_num].push_back(toupper((int)*ptr));
        else
            genre[genre_num].push_back(*ptr);

        ++ptr;
    }
    genre[genre_num].push_back('\0');

    delete [] data;

    return true;
}

const char * GameTDB::RatingToString(int rating)
{
    switch(rating)
    {
        case 0:
            return "CERO";
        case 1:
            return "ESRB";
        case 2:
            return "PEGI";
        default:
            break;
    }

	return NULL;
}

int GameTDB::GetRating(const char * id)
{
    int rating = -1;

    if(!id) return rating;

    char * data = GetGameNode(id);
    if(!data) return rating;

    char * rating_text = GetNodeText(data, "<rating type=\"", "/>");
    if(!rating_text)
    {
        delete [] data;
        return rating;
    }

    if(strncmp(rating_text, "CERO", 4) == 0)
        rating = 0;

    else if(strncmp(rating_text, "ESRB", 4) == 0)
        rating = 1;

    else if(strncmp(rating_text, "PEGI", 4) == 0)
        rating = 2;

    delete [] data;

    return rating;
}

bool GameTDB::GetRatingValue(const char * id, string & rating_value)
{
    if(!id) return false;

    char * data = GetGameNode(id);
    if(!data) return false;

    char * rating_text = GetNodeText(data, "<rating type=\"", "/>");
    if(!rating_text)
    {
        delete [] data;
        return false;
    }

    char * value_text = GetNodeText(rating_text, "value=\"", "\"");
    if(!value_text)
    {
        delete [] data;
        return false;
    }

    rating_value = value_text;

    delete [] data;

    return true;
}

int GameTDB::GetRatingDescriptors(const char * id, safe_vector<string> & desc_list)
{
    if(!id)
        return -1;

    char * data = GetGameNode(id);
    if(!data)
        return -1;

    char * descriptor_text = GetNodeText(data, "<descriptor>", "</rating>");
    if(!descriptor_text)
    {
        delete [] data;
        return -1;
    }

    unsigned int list_num = 0;
    desc_list.clear();

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

    delete [] data;

    return desc_list.size();
}

int GameTDB::GetWifiPlayers(const char * id)
{
    int players = -1;

    if(!id)
        return players;

    char * data = GetGameNode(id);
    if(!data)
        return players;

    char * PlayersNode = GetNodeText(data, "<wi-fi players=\"", "\">");
    if(!PlayersNode)
    {
        delete [] data;
        return players;
    }

    players = atoi(PlayersNode);

    return players;
}

int GameTDB::GetWifiFeatures(const char * id, safe_vector<string> & feat_list)
{
    if(!id)
        return -1;

    char * data = GetGameNode(id);
    if(!data)
        return -1;

    char * feature_text = GetNodeText(data, "<feature>", "</wi-fi>");
    if(!feature_text)
    {
        delete [] data;
        return -1;
    }

    unsigned int list_num = 0;
    feat_list.clear();

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

    delete [] data;

    return feat_list.size();
}

int GameTDB::GetPlayers(const char * id)
{
    int players = -1;

    if(!id)
        return players;

    char * data = GetGameNode(id);
    if(!data)
        return players;

    char * PlayersNode = GetNodeText(data, "<input players=\"", "\">");
    if(!PlayersNode)
    {
        delete [] data;
        return players;
    }

    players = atoi(PlayersNode);

    return players;
}

int GameTDB::GetAccessories(const char * id, safe_vector<Accessory> & acc_list)
{
    if(!id)
        return -1;

    char * data = GetGameNode(id);
    if(!data)
        return -1;

    char * ControlsNode = GetNodeText(data, "<control type=\"", "</input>");
    if(!ControlsNode)
    {
        delete [] data;
        return -1;
    }

    unsigned int list_num = 0;
    acc_list.clear();

    while(ControlsNode && *ControlsNode != '\0')
    {
        if(list_num >= acc_list.size())
            acc_list.resize(list_num+1);

        for(const char * ptr = ControlsNode; *ptr != '"' && *ptr != '\0'; ptr++)
            acc_list[list_num].Name.push_back(*ptr);

        acc_list[list_num].Name.push_back('\0');

        char * requiredField = strstr(ControlsNode, "required=\"");
        if(!requiredField)
        {
            delete [] data;
            return -1;
        }

        requiredField += strlen("required=\"");

        acc_list[list_num].Required = strncmp(requiredField, "true", 4) == 0;

        ControlsNode = strstr(requiredField, "<control type=\"");
        if(ControlsNode)
            ControlsNode += strlen("<control type=\"");

        list_num++;
    }

    delete [] data;

    return acc_list.size();
}

unsigned int GameTDB::FindCaseColor(char * data)
{
	unsigned int color = -1;
	
    char * ColorNode = GetNodeText(data, "<case color=\"", "\"/>");
    if(!ColorNode) return color;

	char format[8];
	sprintf(format, "0x%s", ColorNode);

    return strtoul(format, NULL, 16);
}

unsigned int GameTDB::GetCaseColor(const char * id)
{
    unsigned int color = -1;
    if(!id) return color;

    char * data = GetGameNode(id);
    if(!data) return color;

    color = FindCaseColor(data);
	
	delete [] data;
	return color;
}

bool GameTDB::GetGameXMLInfo(const char * id, GameXMLInfo * gameInfo)
{
    if(!id || !gameInfo)
        return false;

	gameInfo->GameID = id;

    GetTitle(id, gameInfo->Title);
    GetSynopsis(id, gameInfo->Synopsis);
    GetRegion(id, gameInfo->Region);
    GetDeveloper(id, gameInfo->Developer);
    GetPublisher(id, gameInfo->Publisher);
    gameInfo->PublishDate = GetPublishDate(id);
    GetGenres(id, gameInfo->Genres);
    gameInfo->RatingType = GetRating(id);
    GetRatingValue(id, gameInfo->RatingValue);
    GetRatingDescriptors(id, gameInfo->RatingDescriptors);
    gameInfo->WifiPlayers = GetWifiPlayers(id);
    GetWifiFeatures(id, gameInfo->WifiFeatures);
    gameInfo->Players = GetPlayers(id);
    GetAccessories(id, gameInfo->Accessories);
    gameInfo->CaseColor = GetCaseColor(id);

    return true;
}

bool GameTDB::IsLoaded()
{
	return isLoaded;
}
