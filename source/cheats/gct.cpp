#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include "gct.h"

#define ERRORRANGE "Error: CheatNr out of range"

GCTCheats::GCTCheats(void)
{
	Reset();
}

GCTCheats::~GCTCheats(void)
{
	string sGameID ="";
	string sGameTitle = "";
}

unsigned int GCTCheats::getCnt()
{
	return iCntCheats;
}

string GCTCheats::getGameName(void)
{
	return sGameTitle;
}

string GCTCheats::getGameID(void)
{
	return sGameID;
}

string GCTCheats::getCheat(unsigned int nr)
{
	if (nr <= (iCntCheats-1))
		return sCheats[nr];
	return ERRORRANGE;
}

string GCTCheats::getCheatName(unsigned int nr)
{
	if (nr <= (iCntCheats-1))
		return sCheatName[nr];
	return ERRORRANGE;
}

string GCTCheats::getCheatComment(unsigned int nr)
{
	if (nr <= (iCntCheats-1))
		return sCheatComment[nr];
	return ERRORRANGE;
}

int GCTCheats::createGCT(unsigned int nr,const char * filename)
{
	if (nr == 0)return 0;

	ofstream filestr;
	filestr.open(filename);
	if (filestr.fail()) return 0;

	//Header and Footer
	char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
	char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	string buf = getCheat(nr);
	filestr.write(header,sizeof(header));

	int x = 0;
	while (x < (int)buf.size())
	{
		string temp = buf.substr(x,2);
		 long int li = strtol(temp.c_str(),NULL,16);
		temp = li;
		filestr.write(temp.c_str(),1);
		x +=2;
	}
	filestr.write(footer,sizeof(footer));
	filestr.close();

	return 1;
}

int GCTCheats::createGCT(const char * chtbuffer,const char * filename)
{
	ofstream filestr;
	filestr.open(filename);
	if (filestr.fail()) return 0;

	//Header and Footer
	char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
	char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	string buf = chtbuffer;
	filestr.write(header,sizeof(header));

	int x = 0;
	while (x < (int)buf.size())
	{
		string temp = buf.substr(x,2);
		long int li = strtol(temp.c_str(),NULL,16);
		temp = li;
		filestr.write(temp.c_str(),1);
		x +=2;
	}
	filestr.write(footer,sizeof(footer));
	filestr.close();

	return 1;
}

int GCTCheats::createGCT(int nr[],int cnt,const char * filename)
{
	if (cnt == 0) return 0;

	ofstream filestr;
	filestr.open(filename);
	if (filestr.fail()) return 0;

	//Header and Footer
	char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
	char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	filestr.write(header,sizeof(header));

	int c = 0;
	while (c != cnt)
	{
		string buf = getCheat(nr[c]);
		
		int x = 0;
		while (x < (int)buf.size())
		{
			string temp = buf.substr(x,2);
			long int li = strtol(temp.c_str(),NULL,16);
			temp = li;
			filestr.write(temp.c_str(),1);
			x +=2;
		}
		c++;
	}
	filestr.write(footer,sizeof(footer));
	filestr.close();

	return 1;
}

//creates gct from internal array
int GCTCheats::createGCT(const char * filename)
{
	ofstream filestr;
	filestr.open(filename);
	if (filestr.fail()) return 0;

	//Header and Footer
	char header[] = { 0x00, 0xd0, 0xc0, 0xde, 0x00, 0xd0, 0xc0, 0xde};
	char footer[] = { 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	filestr.write(header,sizeof(header));

	for (unsigned int i=0; i < iCntCheats; ++i)
		if (sCheatSelected[i] == true)
		{
			// cheat is selected, export it
			string buf = getCheat(i);

			int x = 0;
			while (x < (int)buf.size())
			{
				string temp = buf.substr(x,2);
				long int li = strtol(temp.c_str(),NULL,16);
				temp = li;
				filestr.write(temp.c_str(),1);
				x +=2;
			}
		}
	filestr.write(footer,sizeof(footer));
	filestr.close();

	return 1;
}

//creates txt from internal array
int GCTCheats::createTXT(const char * filename)
{

	// save gct file
	fstream file;
	file.open(filename,ios::out);

	file << sGameID << endl;
	file << sGameTitle << endl << endl;

	for (unsigned int i=0; i < iCntCheats; ++i)
		if  (sCheatSelected[i])
		{
			file << sCheatName[i] << endl;
			for (unsigned int j=0; j+8 < sCheats[i].size(); j+=16)
				file << sCheats[i].substr(j,8) << " " << sCheats[i].substr(j+8,8) << endl;

			file << "#selected#" << sCheatComment[i] << endl;
			file << endl;
		}
			
	for (unsigned int i=0; i < iCntCheats; ++i)
		if  (!sCheatSelected[i])
		{
			file << sCheatName[i] << endl;
			for (unsigned int j=0; j+8 < sCheats[i].size(); j+=16)
				file << sCheats[i].substr(j,8) << " " << sCheats[i].substr(j+8,8) << endl;

			if (sCheatComment[i].size() > 1)
				file << sCheatComment[i] << endl;
			file << endl;
		}

	file.close();
	return 1;
}


int GCTCheats::openTxtfile(const char * filename)
{
	Reset();

	ifstream filestr;
	filestr.open(filename);
	if (filestr.fail()) return 0;

	int i = 0;
	string str;
	int codestatus;
	bool codedynamic = false; // cheat contains X-Codes?

	filestr.seekg(0,ios_base::end);
	int size = filestr.tellg();
	if (size <= 0) return -1;
	filestr.seekg(0,ios_base::beg);

	getline(filestr,sGameID);
	if (sGameID[sGameID.length() - 1] == '\r')
		sGameID.erase(sGameID.length() - 1);
	
	getline(filestr,sGameTitle);
	if (sGameTitle[sGameTitle.length() - 1] == '\r')
		sGameTitle.erase(sGameTitle.length() - 1);
				
	getline(filestr,sCheatName[i]);  // skip first line if file uses CRLF
	if (!sGameTitle[sGameTitle.length() - 1] == '\r')
	   filestr.seekg(0,ios_base::beg);

	while (!filestr.eof()) {
		getline(filestr,sCheatName[i]); // '\n' delimiter by default
		if (sCheatName[i][sCheatName[i].length() - 1] == '\r')
			sCheatName[i].erase(sCheatName[i].length() - 1);

		string cheatdata;
		bool emptyline = false;

		while (!emptyline)
		{
			getline(filestr,str);
			if (str[str.length() - 1] == '\r')
				str.erase(str.length() - 1);
				 
			if (str == "" || str[0] == '\r' || str[0] == '\n')
			{
				emptyline = true;
				break;
			}

			codestatus = IsCodeEx(str);
			if (codestatus == 1) 
				// line contains X code, so whole cheat is dynamic
				codedynamic = true;
				
			if (codestatus == 2)
			{
				// remove any garbage (comment) after code
				while (str.size() > 17)
					str.erase(str.length() - 1);

				cheatdata.append(str);
				size_t found=cheatdata.find(' ');
				cheatdata.replace(found,1,"");
			}
			else
				sCheatComment[i] = str;

			if (filestr.eof()) break;
		   
		}

		if (!codedynamic && cheatdata.size() > 0)
		{
			sCheats[i] = cheatdata;
			sCheatSelected[i] = false;
			// if comment starts with dynamic, it is selected
			if (sCheatComment[i].compare(0,10,"#selected#") == 0)
			{
				sCheatSelected[i] = true;
				sCheatComment[i].erase(0,10);
			}
			i++;
		}
		else
			sCheatComment[i] = "";
			
		codedynamic = false;
		if (i == MAXCHEATS) break;
	}
	iCntCheats = i;
	filestr.close();
	return 1;
}

bool GCTCheats::IsCode(const std::string& str)
{
	if (str[8] == ' ' && str.size() >= 17)
	{
		// accept strings longer than 17 in case there is a comment on the same line as the code
		char part1[9];
		char part2[9];
		snprintf(part1,sizeof(part1),"%c%c%c%c%c%c%c%c",str[0],str[1],str[2],str[3],str[4],str[5],str[6],str[7]);
		snprintf(part1,sizeof(part2),"%c%c%c%c%c%c%c%c",str[9],str[10],str[11],str[12],str[13],str[14],str[15],str[16]);
		if ((strtok(part1,"0123456789ABCDEFabcdef") == NULL) && (strtok(part2,"0123456789ABCDEFabcdef") == NULL))
			return true;
	}
	return false;
}


int GCTCheats::IsCodeEx(const std::string& str)
{
	int status = 2;
	if (str[8] == ' ' && str.size() >= 17)
	{
		// accept strings longer than 17 in case there is a comment on the same line as the code
		for (int i = 0; i < 17; ++i)
		{
			if (!(str[i] == '0' || str[i] == '1' || str[i] == '2' || str[i] == '3' ||
				  str[i] == '4' || str[i] == '5' || str[i] == '6' || str[i] == '7' ||
				  str[i] == '8' || str[i] == '9' || str[i] == 'a' || str[i] == 'b' ||
				  str[i] == 'c' || str[i] == 'd' || str[i] == 'e' || str[i] == 'f' ||
				  str[i] == 'A' || str[i] == 'B' || str[i] == 'C' || str[i] == 'D' ||
				  str[i] == 'E' || str[i] == 'F' || str[i] == 'x' || str[i] == 'X') && i!=8 )
				status  = 0; // not even x -> no code
			if ((str[i] == 'x' || str[i] == 'X') && i!=8 && status >= 1)
				status  = 1;
		}
		return status;
	}
	return 0; // no code
}

void GCTCheats::Reset()
{
	iCntCheats = 0;
	for (int i=0;i<MAXCHEATS;++i) 
		sCheatSelected[i] = false;
}