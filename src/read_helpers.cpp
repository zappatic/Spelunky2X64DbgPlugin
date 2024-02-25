#include "read_helpers.h"

std::string S2Plugin::ReadConstString(uintptr_t addr)
{
    return ReadConstBasicString<char>(addr);
};
