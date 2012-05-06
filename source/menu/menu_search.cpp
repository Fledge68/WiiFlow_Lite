#include "menu.hpp"
#include "gecko.h"
#include <stdlib.h>

// Returns a list of games which starts with the specified (partial) gameId
// We can enhance the code in this file later on to support more search features
// Using a search class as argument or something like that
vector<dir_discHdr> CMenu::_searchGamesByID(const char *gameId)
{
	vector<dir_discHdr> retval;
	for (vector<dir_discHdr>::iterator itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if (strncmp((const char *) (*itr).hdr.id, gameId, strlen(gameId)) == 0)
			retval.push_back(*itr);

	return retval;
}
/* 
vector<dir_discHdr> CMenu::_searchGamesByTitle(wchar_t letter)
{
	vector<dir_discHdr> retval;
	for (vector<dir_discHdr>::iterator itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if ((*itr).title[0] == letter)
			retval.push_back(*itr);

	return retval;
}

vector<dir_discHdr> CMenu::_searchGamesByType(const char type)
{
	vector<dir_discHdr> retval;
	for (vector<dir_discHdr>::iterator itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if ((*itr).id[0] == type)
			retval.push_back(*itr);

	return retval;
}

vector<dir_discHdr> CMenu::_searchGamesByRegion(const char region)
{
	vector<dir_discHdr> retval;
	for (vector<dir_discHdr>::iterator itr = m_gameList.begin(); itr != m_gameList.end(); itr++)
		if ((*itr).id[3] == region)
			retval.push_back(*itr);

	return retval;
} */