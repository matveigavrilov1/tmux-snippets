#include "browser/storageBrowser.h"
#include "data/xmlStorageManager.h"
#include "utils/exePathManager.h"
#include "utils/finally.h"

#include <string>
#include <filesystem>

int main(int argc, char* argv[])
{
	std::string paneToSendSnippet(argv[1]);

	utils::exePathManager::getInstance().initialize(argv[0]);
	data::xmlStorageManager xmlStorage;
	xmlStorage.parse(utils::exePathManager::getInstance().getStoragePath());
	auto xmlStorageDumpCallback = [&xmlStorage]()
	{
		xmlStorage.dump(utils::exePathManager::getInstance().getStoragePath());
	};
	utils::finally xmlStorageDump(xmlStorageDumpCallback);

	ui::runStorageBrowser(xmlStorage.getStorage(), paneToSendSnippet);

	return 0;
}