// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <daScript/misc/platform.h>

#ifdef foreach
#define NOGGIT_FOREACH_WAS_DEFINED
#undef foreach
#endif

#include <daScript/ast/ast.h>

#ifdef NOGGIT_FOREACH_WAS_DEFINED
#define foreach Q_FOREACH
#undef NOGGIT_FOREACH_WAS_DEFINED
#endif
