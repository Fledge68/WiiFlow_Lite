
#ifndef __WSTRINGEX_HPP
#define __WSTRINGEX_HPP

#include <string>
using namespace std;

class wstringEx : public basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >
{
public:
	wstringEx(void) { }
	wstringEx(const wchar_t *s);
	wstringEx(const basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> > &ws);
	wstringEx(const string &s);
	wstringEx &operator=(const string &s);
	void fromUTF8(const string &s);
	string toUTF8(void) const;
};

#endif // !defined(__WSTRINGEX_HPP)

