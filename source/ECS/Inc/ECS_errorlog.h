#ifndef MYECS_ERRORLOG_H
#define MYECS_ERRORLOG_H

#include <string>
#include <typeinfo>
#include <fmt/core.h>
#include <experimental/source_location>

namespace MyECS::Debug
{
    template<char ...string>
    using const_string = std::integer_sequence<char, string...>;

    template<typename T, T ...string>
    constexpr const_string<string...> operator""_cStr() { return {}; }

    template<typename> struct ConstStr;

    template<char ...string>
    struct ConstStr<const_string<string...>>
    {
        const char* Get() const
        {
            static constexpr char str[sizeof...(string) + 1] = {string..., '\0'};
            return str;
        }
    };

    template<ConstStr str, typename ...Args>
    static void ECS_errorlog(Args... args)
    {
        constexpr auto loc = std::experimental::source_location::current();

        fmt::print("| [{}] {}:{}r,{}c | [ECS] [error] ", loc.file_name(), loc.function_name(), loc.line(), loc.column());
        fmt::print(str.Get(), args...);
    }
}
#endif
