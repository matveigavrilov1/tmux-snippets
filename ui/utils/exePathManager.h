#pragma once

#include <filesystem>

namespace utils
{
class exePathManager
{
private:
	std::filesystem::path exePath;
	std::filesystem::path exeDir;
	bool initialized = false;
	exePathManager() = default;

public:
	exePathManager(const exePathManager&) = delete;
	exePathManager& operator= (const exePathManager&) = delete;

	static exePathManager& getInstance();

	void initialize(const char* argv0);
	const std::filesystem::path& getExePath() const;
	const std::filesystem::path& getExeDir() const;
	std::filesystem::path getStoragePath() const;
	std::filesystem::path getFileSnippetPath(const std::string& filename) const;
	bool isInitialized() const;
};
} // namespace utils