
#ifndef __WSTRINGEX_HPP
#define __WSTRINGEX_HPP

#include <string>

class wstringEx : public std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >
{
public:
	wstringEx(void) { }
	wstringEx(const wchar_t *s);
	wstringEx(const std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> > &ws);
	wstringEx(const std::string &s);
	wstringEx &operator=(const std::string &s);
	void fromUTF8(const char *s);
	std::string toUTF8(void) const;
};


#endif // !defined(__WSTRINGEX_HPP)

