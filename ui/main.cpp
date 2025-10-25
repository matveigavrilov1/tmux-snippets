#include "browser/storageBrowser.h"

int main(int argc, char* argv[])
{
	std::string pane(argv[1]);
	auto storage = std::make_shared<data::storage>();

	auto folder1_uuid = storage->addFolder("Projects");
	auto folder2_uuid = storage->addFolder("Work");
	storage->addSnippet(pane, pane);

	std::string snippet;
	storage->folderDown(folder1_uuid);
	storage->addSnippet("C++ Code", "#include <iostream>");
	ui::runStorageBrowser(storage, pane);

	return 0;
}