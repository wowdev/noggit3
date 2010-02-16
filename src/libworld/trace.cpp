#include "trace.h"

std::string cm::Holder::trace_file_  = "";
int         cm::Holder::depth_       = 0;

const char* cm::Holder::nest_        = "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ";
time_t      cm::Holder::last_invoke_time_ = 0;
bool		cm::Holder::isDEEP		= false;
