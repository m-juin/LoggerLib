#ifndef __LOGGERLIB_UTILS_HPP__
#define __LOGGERLIB_UTILS_HPP__

namespace LoggerLib::Utils
{
    namespace Color
    {
        inline const char *NONE = "\033[0m";
        inline const char *RED = "\033[31;1m";
        inline const char *WHITE = "\033[37;1m";
        inline const char *GREEN = "\033[32;1m";
        inline const char *YELLOW = "\033[33;1m";
    } // namespace Color

}

#endif // __LOGGERLIB_UTILS_HPP__
