/**
 * @file diagnostic_code.cpp
 * @brief 诊断代码转换实现
 * @author BegoniaHe
 */

#include "czc/diagnostics/diagnostic_code.hpp"
#include <sstream>
#include <iomanip>

std::string diagnostic_code_to_string(DiagnosticCode code)
{
    int code_num = static_cast<int>(code);
    char prefix;
    int offset = 0;

    if (code_num < 1000)
    {
        prefix = 'L';
        offset = code_num;
    }
    else if (code_num < 2000)
    {
        prefix = 'T';
        offset = code_num - 1000;
    }
    else
    {
        return "U0000";
    }

    std::ostringstream oss;
    oss << prefix << std::setw(4) << std::setfill('0') << offset;
    return oss.str();
}
