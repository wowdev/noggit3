// trace.h by schlumpf

#ifndef TRACE_H
#define TRACE_H

#define _DEEP true

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <string>

#define _TF(trace_file)	cm::Holder::LogToFile(trace_file)
#define _T cm::Trace __CM_TRACE__
#define _TD cm::TraceDeep __CM_TRACE__
#define _Tn(func_name) cm::Trace __CM_TRACE__(func_name, "()")
#define _TDn(func_name) cm::TraceDeep __CM_TRACE__(func_name, "()")

namespace	cm
{
	class Holder
	{
	public:
		void static LogMsg(int depth, int align, const char *fmt, va_list args)
		{
			FILE	*fp = fopen(trace_file_.c_str(), "a+");
			if (fp == NULL)
			{
				return;
			}


			time_t		curTime;
			time(&curTime);

			char	timeStamp[32] = { 0 };
			strftime(timeStamp, sizeof(timeStamp), "%Y%m%d.%H%M%S", localtime(&curTime));

			// only log the timestamp when the time changes

			unsigned int len = fprintf( fp, "%s %*.*s> (%d)",
					(last_invoke_time_ != curTime) ? timeStamp : "               ",
					2 * depth,
					2 * depth,
					nest_,
					depth);
			last_invoke_time_ = curTime;
			len += vfprintf(fp, fmt, args);
			len += fwrite("\n", 1, 1, fp);
			fflush(fp);
			fclose(fp);
		}

		void static setDeep(bool _isDEEP)
		{
			isDEEP = _isDEEP;
		}

		void static LogToFile(const char *trace_file)
		{
			trace_file_ = trace_file;
			FILE	*fp = fopen(trace_file_.c_str(), "w");
			fclose(fp);
		}

		static std::string	trace_file_;

		// function call stack depth

		static int			depth_;
		static const char*  nest_;
		static time_t       last_invoke_time_;

		static bool			isDEEP;
	};

	class	TraceDeep
    {
    public:
    	explicit TraceDeep(char *func_name, const char* argsfmt, ...)
    	{
			if(!cm::Holder::isDEEP)
			{
				++cm::Holder::depth_;
				return;
			}
            char fmt[256] ={0};
            sprintf(fmt, "%s%s", func_name, argsfmt);
    	    va_list arglist;
    	    va_start(arglist, argsfmt);
			cm::Holder::LogMsg(cm::Holder::depth_, cm::Holder::depth_ * 2, fmt,  arglist);
    		va_end(arglist);
    		++cm::Holder::depth_;
    	}

    	~TraceDeep()
    	{
    		--cm::Holder::depth_;
    	}
    };
	class	Trace
    {
    public:
    	explicit Trace(char *func_name, const char* argsfmt, ...)
    	{
            char fmt[256] ={0};
            sprintf(fmt, "%s%s", func_name, argsfmt);
    	    va_list arglist;
    	    va_start(arglist, argsfmt);
			cm::Holder::LogMsg(cm::Holder::depth_, cm::Holder::depth_ * 2, fmt,  arglist);
    		va_end(arglist);
    		++cm::Holder::depth_;
    	}

    	~Trace()
    	{
    		--cm::Holder::depth_;
    	}
    };
}	// end namespace cm

#endif
