#include <dukglue.h>
#include <noggit/scripting/script_printf.hpp>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_loader.hpp>

using namespace noggit::scripting;

/**
 * From: https://github.com/svaarala/duktape/issues/1565 
 * 
 * Authors comment:
 * A very simple format function similar to printf which does not support any of the flags, width, precision or length
 * parameters that printf can handle.
 * Modeled after https://github.com/nodejs/node/blob/master/lib/util.js#L65.
 */
duk_ret_t noggit::scripting::print(duk_context *ctx)
{
    // Stack can vary here. There can be no format string or the format string doesn't contain (enough) format specifiers
    // for all parameters. In that case they are simply stringified and appended to the result string.
    // stack: [ optional-format-string <varargs> ] but at least one of the 2 must be present.
    duk_int_t top = duk_get_top(ctx);
    if (top == 0)
    {
        duk_error(ctx, DUK_ERR_TYPE_ERROR, "format string expected");
        return 0;
    }

    std::string result;

    if (top == 1 || !duk_is_string(ctx, 0))
    {
        // No format string or no additional parameters.
        for (duk_int_t i = 0; i < top; ++i)
        {
            if (!result.empty())
                result += " ";
            result += duk_to_string(ctx, i);
        }
    }
    else
    {
        std::string format = duk_get_string(ctx, 0);
        duk_int_t currentArgument = 1;
        duk_int_t lastPosition = 0;

        for (duk_int_t i = 0; i < format.size();)
        {
            if (format[i] == '%' && (i + 1 < format.size()))
            {
                switch (format[i + 1])
                {
                case 'd':
                case 'f':
                    if (currentArgument >= top)
                        break;
                    if (lastPosition < i)
                        result += format.substr(lastPosition, i - lastPosition);

                    // Make the argument (might be a string, object, null, number etc.) into a number.
                    // Then convert that to a string for the result.
                    duk_to_number(ctx, currentArgument);
                    result += duk_to_string(ctx, currentArgument++);
                    lastPosition = i = i + 2;
                    continue;

                case 'j':
                    if (currentArgument >= top)
                        break;
                    if (lastPosition < i)
                        result += format.substr(lastPosition, i - lastPosition);

                    result += duk_json_encode(ctx, currentArgument++);
                    lastPosition = i = i + 2;
                    continue;

                case 's':
                    if (currentArgument >= top)
                        break;
                    if (lastPosition < i)
                        result += format.substr(lastPosition, i - lastPosition);
                    result += duk_get_string(ctx, currentArgument++);
                    lastPosition = i = i + 2;
                    continue;

                case '%':
                    if (lastPosition < i)
                        result += format.substr(lastPosition, i - lastPosition);
                    result += '%';
                    lastPosition = i = i + 2;
                    continue;
                }
            }
            ++i;
        }

        if (lastPosition == 0)
            result = format;
        else if (lastPosition < format.size())
            result += format.substr(lastPosition);

        while (currentArgument < top)
        {
            result += std::string(" ") + duk_to_string(ctx, currentArgument++);
        }
    }

    if (get_cur_tool())
        get_cur_tool()->addLog("[log]: "+result);

    return 1;
}

void noggit::scripting::register_print_functions(duk_context* ctx)
{
    TAPE_FUNCTION(ctx,print,-1);
}