// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <daScript/daScriptModule.h>

//! \note Needs to be used in the global namespace.
#define DAS_FORWARD_DECLARE_MODULE(module_name_) \
  extern das::Module* register_ ## module_name_ (); \

//! \note Requires DAS_FORWARD_DECLARE_MODULE (or an include actually
//! declaring the module) to have happened before.
#define DAS_NEED_MODULE(module_name_) \
    das::ModuleKarma += unsigned (intptr_t (::register_ ## module_name_()))
