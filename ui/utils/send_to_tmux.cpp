
#include <string>
#include <cstdio>
#include <memory>
#include <sstream>
#include <fstream>

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

static std::string escapeSingleQuotes(const std::string& input)
{
	std::string result;
	for (char c : input)
	{
		if (c == '\'')
		{
			result += "'\\''";
		}
		else
		{
			result += c;
		}
	}
	return result;
}

void utils::sendCommandToTmux(const std::string& command, const std::string& target)
{
	std::istringstream stream(command);
	std::string line;
	std::string fullCommand = "tmux send-keys -t " + target;

	while (std::getline(stream, line))
	{
		if (!line.empty())
		{
			fullCommand += " '" + escapeSingleQuotes(line) + "' Enter";
		}
	}

	executeCommand(fullCommand);
}