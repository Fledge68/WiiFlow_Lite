#include <string.h>
#include <malloc.h>

#include "gcard.h"
#include "http.h"
#include "loader/utils.h"
#include "gecko/gecko.h"

#define MAX_URL_SIZE 178 // 128 + 48 + 6

struct provider
{
	char url[128];
	char key[48];
};

struct provider *providers = NULL;
int amount_of_providers = 0;

u8 register_card_provider(const char *url, const char *key)
{
	if (strlen(url) > 0 && strlen(key) > 0 && strstr(url, "{KEY}") != NULL && strstr(url, "{ID6}") != NULL)
	{
		providers = (struct provider *) realloc(providers, (amount_of_providers + 1) * sizeof(struct provider));
		memset(&providers[amount_of_providers], 0, sizeof(struct provider));
		strncpy((char *) providers[amount_of_providers].url, url, 128);
		strncpy((char *) providers[amount_of_providers].key, key, 48);
		amount_of_providers++;
		gprintf("Gamercard provider is valid!\n");
		return 0;
	}
	gprintf("Gamertag provider is NOT valid!\n");
	return -1;
}

u8 has_enabled_providers()
{
	if (amount_of_providers != 0 && providers != NULL)
		return 1;
	return 0;
}

void add_game_to_card(const char *gameid)
{
	int i;

	char *url = (char *)malloc(MAX_URL_SIZE); // Too much memory, but only like 10 bytes
	memset(url, 0, sizeof(url));

	for (i = 0; i < amount_of_providers && providers != NULL; i++)
	{
		strcpy(url, (char *) providers[i].url);
		str_replace(url, (char *) "{KEY}", (char *) providers[i].key, MAX_URL_SIZE);		
		str_replace(url, (char *) "{ID6}", (char *) gameid, MAX_URL_SIZE);

		gprintf("Gamertag URL:\n%s\n",(char*)url);
		downloadfile(NULL, 0, (char *) url, NULL, NULL);
	}
	free(url);
}