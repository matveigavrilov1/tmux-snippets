#include "utils/exePathManager.h"

namespace utils
{
exePathManager& exePathManager::getInstance()
{
	static exePathManager instance;
	return instance;
}

void exePathManager::initialize(const char* argv0)
{
	exePath = std::filesystem::path(argv0);
	exeDir = exePath.parent_path();
	initialized = true;
}

const std::filesystem::path& exePathManager::getExePath() const
{
	if (!initialized)
	{
		throw std::runtime_error("ExePathManager not initialized");
	}
	return exePath;
}

const std::filesystem::path& exePathManager::getExeDir() const
{
	if (!initialized)
	{
		throw std::runtime_error("ExePathManager not initialized");
	}
	return exeDir;
}

std::filesystem::path exePathManager::getStoragePath() const
{
	return getExeDir() / "data" / "storage.xml";
}

std::filesystem::path exePathManager::getFileSnippetPath(const std::string& filename) const
{
	std::filesystem::path filePath(filename);


	if (filePath.is_absolute())
	{
		return filePath;
	}

	return getExeDir() / "data" / filename;
}

bool exePathManager::isInitialized() const
{
	return initialized;
}
} // namespace utils