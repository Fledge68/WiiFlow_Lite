#include "text.hpp"

using namespace std;

static const wchar_t *g_whitespaces = L" \f\n\r\t\v";

// Simplified use of sprintf
const char *fmt(const char *format, ...)
{
	enum {
		MAX_MSG_SIZE	= 512,
		MAX_USES		= 8
	};

	static int currentStr = 0;
	currentStr = (currentStr + 1) % MAX_USES;

	va_list va;
	va_start(va, format);
	static char buffer[MAX_USES][MAX_MSG_SIZE];
	vsnprintf(buffer[currentStr], MAX_MSG_SIZE, format, va);
	buffer[currentStr][MAX_MSG_SIZE - 1] = '\0';
	va_end(va);

	return buffer[currentStr];
}

string sfmt(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	u32 length = vsnprintf(0, 0, format, va) + 1;
	va_end(va);
	char *tmp = new char[length + 1];
	va_start(va, format);
	vsnprintf(tmp, length, format, va);
	va_end(va);
	string s = tmp;
	delete[] tmp;
	return s;
}

static inline bool fmtCount(const wstringEx &format, int &i, int &s)
{
	int state = 0;

	i = 0;
	s = 0;
	for (u32 k = 0; k < format.size(); ++k)
	{
		if (state == 0)
		{
			if (format[k] == L'%')
				state = 1;
		}
		else if (state == 1)
		{
			switch (format[k])
			{
				case L'%':
					state = 0;
					break;
				case L'i':
				case L'd':
					state = 0;
					++i;
					break;
				case L's':
					state = 0;
					++s;
					break;
				default:
					return false;
			}
		}
	}
	return true;
}

// Only handles the cases i need for translations : plain %i and %s
bool checkFmt(const wstringEx &ref, const wstringEx &format)
{
	int s;
	int i;
	int refs;
	int refi;
	if (!fmtCount(ref, refi, refs))
		return false;
	if (!fmtCount(format, i, s))
		return false;
	return i == refi && s == refs;
}

wstringEx wfmt(const wstringEx &format, ...)
{
	// Don't care about performance
	va_list va;
	string f(format.toUTF8());
	va_start(va, format);
	u32 length = vsnprintf(0, 0, f.c_str(), va) + 1;
	va_end(va);
	char *tmp = new char[length + 1];
	va_start(va, format);
	vsnprintf(tmp, length, f.c_str(), va);
	va_end(va);
	wstringEx ws;
	ws.fromUTF8(tmp);
	delete[] tmp;
	return ws;
}

string vectorToString(const safe_vector<string> &vect, string sep)
{
	string s;
	for (u32 i = 0; i < vect.size(); ++i)
	{
		if (i > 0)
			s.append(sep);
		s.append(vect[i]);
	}
	return s;
}

wstringEx vectorToString(const safe_vector<wstringEx> &vect, char sep)
{
	wstringEx s;
	for (u32 i = 0; i < vect.size(); ++i)
	{
		if (i > 0)
			s.push_back(sep);
		s.append(vect[i]);
	}
	return s;
}

safe_vector<string> stringToVector(const string &text, char sep)
{
	safe_vector<string> v;
	if (text.empty()) return v;
	u32 count = 1;
	for (u32 i = 0; i < text.size(); ++i)
		if (text[i] == sep)
			++count;
	v.reserve(count);
	string::size_type off = 0;
	string::size_type i = 0;
	do
	{
		i = text.find_first_of(sep, off);
		if (i != string::npos)
		{
			string ws(text.substr(off, i - off));
			v.push_back(ws);
			off = i + 1;
		}
		else
			v.push_back(text.substr(off));
	} while (i != string::npos);
	return v;
}

safe_vector<wstringEx> stringToVector(const wstringEx &text, char sep)
{
	safe_vector<wstringEx> v;
	if (text.empty()) return v;
	u32 count = 1;
	for (u32 i = 0; i < text.size(); ++i)
		if (text[i] == sep)
			++count;
	v.reserve(count);
	wstringEx::size_type off = 0;
	wstringEx::size_type i = 0;
	do
	{
		i = text.find_first_of(sep, off);
		if (i != wstringEx::npos)
		{
			wstringEx ws(text.substr(off, i - off));
			v.push_back(ws);
			off = i + 1;
		}
		else
			v.push_back(text.substr(off));
	} while (i != wstringEx::npos);
	return v;
}

bool SFont::fromBuffer(const SmartBuf &buffer, u32 bufferSize, u32 size, u32 lspacing, u32 w, u32 idx, const char *)
{
	if (!buffer || !font) return false;

	size = min(max(6u, size), 1000u);
	lineSpacing = min(max(6u, lspacing), 1000u);
	weight = min(w, 32u);
	index = idx;

	SMART_FREE(data);
	data = smartMem2Alloc(bufferSize);
	if(!data) return false;

	memcpy(data.get(), buffer.get(), bufferSize);
	dataSize = bufferSize;

	font->loadFont(data.get(), dataSize, size, weight, index, false);

	return true;
}

bool SFont::fromFile(const char *filename, u32 size, u32 lspacing, u32 w, u32 idx)
{
	if (!font) return false;
	size = min(max(6u, size), 1000u);
	weight = min(w, 32u);
	index = idx = 0;

	lineSpacing = min(max(6u, lspacing), 1000u);

	FILE *file = fopen(filename, "rb");
	if (file == NULL) return false;
	fseek(file, 0, SEEK_END);
	u32 fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (fileSize == 0) return false;
	
	SMART_FREE(data);
	data = smartMem2Alloc(fileSize);
	if (!data)
	{
		SAFE_CLOSE(file);
		return false;
	}
		
	fread(data.get(), 1, fileSize, file);

	dataSize = fileSize;

	font->loadFont(data.get(), dataSize, size, weight, index, false);
	return true;
}

void CText::setText(SFont font, const wstringEx &t)
{
	CText::SWord w;
	m_lines.clear();
	if (!!font.font) m_font = font;
	if (!m_font.font) return;

	firstLine = 0;
	// Don't care about performance
	safe_vector<wstringEx> lines = stringToVector(t, L'\n');
	m_lines.reserve(lines.size());
	// 
	for (u32 k = 0; k < lines.size(); ++k)
	{
		wstringEx &l = lines[k];
		m_lines.push_back(CText::CLine());
		m_lines.back().reserve(32);
		wstringEx::size_type i = l.find_first_not_of(g_whitespaces);
		wstringEx::size_type j;
		while (i != wstringEx::npos)
		{
			j = l.find_first_of(g_whitespaces, i);
			if (j != wstringEx::npos && j > i)
			{
				w.text.assign(l, i, j - i);
				m_lines.back().push_back(w);
				i = l.find_first_not_of(g_whitespaces, j);
			}
			else if (j == wstringEx::npos)
			{
				w.text.assign(l, i, l.size() - i);
				m_lines.back().push_back(w);
				i = wstringEx::npos;
			}
		}
	}
}

void CText::setText(SFont font, const wstringEx &t, u32 startline)
{
	CText::SWord w;
	totalHeight = 0;

	m_lines.clear();
	if (!!font.font) m_font = font;
	if (!m_font.font) return;

	firstLine = startline;
	// Don't care about performance
	safe_vector<wstringEx> lines = stringToVector(t, L'\n');
	m_lines.reserve(lines.size());
	// 
	for (u32 k = 0; k < lines.size(); ++k)
	{
		wstringEx &l = lines[k];
		m_lines.push_back(CText::CLine());
		m_lines.back().reserve(32);
		wstringEx::size_type i = l.find_first_not_of(g_whitespaces);
		wstringEx::size_type j;
		while (i != wstringEx::npos)
		{
			j = l.find_first_of(g_whitespaces, i);
			if (j != wstringEx::npos && j > i)
			{
				w.text.assign(l, i, j - i);
				m_lines.back().push_back(w);
				i = l.find_first_not_of(g_whitespaces, j);
			}
			else if (j == wstringEx::npos)
			{
				w.text.assign(l, i, l.size() - i);
				m_lines.back().push_back(w);
				i = wstringEx::npos;
			}
		}
	}
}


void CText::setFrame(float width, u16 style, bool ignoreNewlines, bool instant)
{
	float shift;

	totalHeight = 0;

	if (!m_font.font) return;

	float space = m_font.font->getWidth(L" ");
	float posX = 0.f;
	float posY = 0.f;
	u32 lineBeg = 0;

	if(firstLine > m_lines.size()) firstLine = 0;

	for (u32 k = firstLine; k < m_lines.size(); ++k)
	{
		CText::CLine &words = m_lines[k];
		if (words.empty())
		{
			posY += (float)m_font.lineSpacing;
			continue;
		}

		for (u32 i = 0; i < words.size(); ++i)
		{
			float wordWidth = m_font.font->getWidth(words[i].text.c_str());
			if (posX == 0.f || posX + (float)wordWidth + space * 2 <= width)
			{
				words[i].targetPos = Vector3D(posX, posY, 0.f);
				posX += wordWidth + space;
			}
			else
			{
				posY += (float)m_font.lineSpacing;
				words[i].targetPos = Vector3D(0.f, posY, 0.f);
				if ((style & (FTGX_JUSTIFY_CENTER | FTGX_JUSTIFY_RIGHT)) != 0)
				{
					posX -= space;
					shift = (style & FTGX_JUSTIFY_CENTER) != 0 ? -posX * 0.5f : -posX;
					for (u32 j = lineBeg; j < i; ++j)
						words[j].targetPos.x += shift;
				}
				posX = wordWidth + space;
				lineBeg = i;
			}
		}
		// Quick patch for newline support
		if (!ignoreNewlines && k + 1 < m_lines.size())
			posX = 9999999.f;
	}
	totalHeight = posY + m_font.lineSpacing;
	
	if ((style & (FTGX_JUSTIFY_CENTER | FTGX_JUSTIFY_RIGHT)) != 0)
	{
		posX -= space;
		shift = (style & FTGX_JUSTIFY_CENTER) != 0 ? -posX * 0.5f : -posX;
		for (u32 k = firstLine; k < m_lines.size(); ++k)
			for (u32 j = lineBeg; j < m_lines[k].size(); ++j)
				m_lines[k][j].targetPos.x += shift;
	}
	if ((style & (FTGX_ALIGN_MIDDLE | FTGX_ALIGN_BOTTOM)) != 0)
	{
		posY += (float)m_font.lineSpacing;
		shift = (style & FTGX_ALIGN_MIDDLE) != 0 ? -posY * 0.5f : -posY;
		for (u32 k = firstLine; k < m_lines.size(); ++k)
			for (u32 j = 0; j < m_lines[k].size(); ++j)
				m_lines[k][j].targetPos.y += shift;
	}
	if (instant)
		for (u32 k = firstLine; k < m_lines.size(); ++k)
			for (u32 i = 0; i < m_lines[k].size(); ++i)
				m_lines[k][i].pos = m_lines[k][i].targetPos;
}

void CText::setColor(const CColor &c)
{
	m_color = c;
}

void CText::tick(void)
{
	for (u32 k = 0; k < m_lines.size(); ++k)
		for (u32 i = 0; i < m_lines[k].size(); ++i)
			m_lines[k][i].pos += (m_lines[k][i].targetPos - m_lines[k][i].pos) * 0.05f;
}

void CText::draw(void)
{
	if (!m_font.font) return;

	for (u32 k = firstLine; k < m_lines.size(); ++k)
		for (u32 i = 0; i < m_lines[k].size(); ++i)
		{
			m_font.font->setX(m_lines[k][i].pos.x);
			m_font.font->setY(m_lines[k][i].pos.y);
			m_font.font->drawText(0, m_font.lineSpacing, m_lines[k][i].text.c_str(), m_color);
		}
}

int CText::getTotalHeight(void)
{
	return totalHeight;
}

string upperCase(string text)
{
	char c;
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'a' && c <= 'z')
			text[i] = c & 0xDF;
	}
	return text;
}


string lowerCase(string text)
{
	char c;
	for (string::size_type i = 0; i < text.size(); ++i)
	{
		c = text[i];
		if (c >= 'A' && c <= 'Z')
			text[i] = c | 0x20;
	}
	return text;
}

// trim from start
string ltrim(string s)
{
	s.erase(s.begin(), find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

// trim from end
string rtrim(string s)
{
	s.erase(find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

void Asciify( wchar_t *str )
{
	const wchar_t *ptr = str;
	wchar_t *ctr = str;
	
	while(*ptr != '\0')
    {
		switch(*ptr)
		{
			case 0x14c:
				*ctr = 0x4f;
				break;
		}
		*ctr = *ptr;
		++ptr;
		++ctr;	
	}
	*ctr = '\0';
}
