
#include <string>
#include <cstdio>
#include <memory>

#include "utils/send_to_tmux.h"

static std::string executeCommand(const std::string& command)
{
	char buffer[128];
	std::string result = "";

	// Открываем pipe для чтения вывода команды
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}

	while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr)
	{
		result += buffer;
	}

	return result;
}

void utils::sendCommandToTmux(const std::string& command, const std::string& target)
{
	std::string fullCommand = "tmux send-keys -t " + target + " '" + command + "' Enter";
	executeCommand(fullCommand);
}