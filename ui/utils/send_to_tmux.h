#pragma once

#include <string>

namespace utils
{
void sendCommandToTmux(const std::string& command, const std::string& target = "0");
}