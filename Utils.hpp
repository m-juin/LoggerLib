#ifndef __LOGGERLIB_UTILS_HPP__
#define __LOGGERLIB_UTILS_HPP__

namespace LoggerLib::Utils
{
    namespace Color
    {
        constexpr char *NONE = "\033[0m";
        constexpr char *RED = "\033[31;1m";
        constexpr char *WHITE = "\033[37;1m";
        constexpr char *GREEN = "\033[32;1m";
        constexpr char *YELLOW = "\033[33;1m";
    } // namespace Color

}

#endif // __LOGGERLIB_UTILS_HPP__