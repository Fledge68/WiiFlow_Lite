
#ifndef _CONST_STR_HPP_
#define _CONST_STR_HPP_

#include "wstringEx/wstringEx.hpp"
#include "gui/text.hpp"
#include "defines.h"
#include "svnrev.h"

static const string &VERSION_STRING = sfmt("%s (%s-r%s)", APP_NAME, APP_VERSION, SVN_REV);
static const wstringEx SVN_REV_W(SVN_REV);

#endif
