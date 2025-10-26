#include "data/xmlStorageManager.h"
#include "utils/generate_uuid.h"

namespace data
{
xmlStorageManager::xmlStorageManager()
: storage_(std::make_shared<storage>())
{ }

storage::shared_ptr_t xmlStorageManager::getStorage() const
{
	return storage_;
}

bool xmlStorageManager::parse(const std::string& filename)
{
	pugi::xml_document doc;
	if (!doc.load_file(filename.c_str()))
	{
		return false;
	}

	auto rootNode = doc.child("storage");
	if (!rootNode)
	{
		return false;
	}

	// Сначала парсим сниппеты корневого уровня
	for (auto snippetNode : rootNode.children("snippet"))
	{
		auto snippet = parseSnippet(snippetNode);
		storage_->root()->snippets_.push_back(snippet);
	}

	// Затем парсим папки
	parseFolder(rootNode, storage_->root());
	return true;
}

bool xmlStorageManager::dump(const std::string& filename)
{
	pugi::xml_document doc;
	auto storageNode = doc.append_child("storage");

	// Сначала дампим сниппеты корневого уровня
	for (const auto& snippet : storage_->root()->snippets_)
	{
		auto snippetNode = storageNode.append_child("snippet");
		dumpSnippet(snippetNode, snippet);
	}

	// Затем дампим папки
	dumpFolder(storageNode, storage_->root());

	if (!doc.save_file(filename.c_str()))
	{
		return false;
	}

	return true;
}

storage::snippet_shared_ptr_t xmlStorageManager::parseSnippet(const pugi::xml_node& snippetNode)
{
	std::string title = snippetNode.child_value("title");
	std::string content = snippetNode.child_value("content");
	std::string uuidStr = snippetNode.attribute("uuid").as_string();
	bool from_file = snippetNode.attribute("from_file").as_bool(false);

	uuids::uuid snippetUuid;
	try
	{
		snippetUuid = uuids::uuid::from_string(uuidStr).value();
	}
	catch (...)
	{
		snippetUuid = utils::generate_uuid();
	}

	auto snippet = std::make_shared<storage::snippet_t>();
	snippet->title = title;
	snippet->content = content;
	snippet->uuid = snippetUuid;
	snippet->from_file = from_file;

	return snippet;
}

void xmlStorageManager::dumpSnippet(pugi::xml_node& snippetNode, const storage::snippet_shared_ptr_t& snippet)
{
	snippetNode.append_attribute("uuid").set_value(uuids::to_string(snippet->uuid).c_str());
	snippetNode.append_attribute("from_file").set_value(snippet->from_file);

	snippetNode.append_child("title").text().set(snippet->title.c_str());
	snippetNode.append_child("content").text().set(snippet->content.c_str());
}

void xmlStorageManager::parseFolder(const pugi::xml_node& xmlNode, std::shared_ptr<storage::folder> folder)
{
	for (auto subFolderNode : xmlNode.children("folder"))
	{
		std::string folderName = subFolderNode.attribute("name").as_string();
		std::string folderUuidStr = subFolderNode.attribute("uuid").as_string();

		uuids::uuid folderUuid;
		try
		{
			folderUuid = uuids::uuid::from_string(folderUuidStr).value();
		}
		catch (...)
		{
			folderUuid = utils::generate_uuid();
		}

		auto newFolder = std::make_shared<storage::folder>(folderName, folderUuid);
		newFolder->parent_ = folder;
		folder->subFolders_[folderUuid] = newFolder;

		// Парсим сниппеты подпапки
		for (auto snippetNode : subFolderNode.children("snippet"))
		{
			auto snippet = parseSnippet(snippetNode);
			newFolder->snippets_.push_back(snippet);
		}

		// Рекурсивно парсим вложенные папки
		parseFolder(subFolderNode, newFolder);
	}
}

void xmlStorageManager::dumpFolder(pugi::xml_node& xmlNode, std::shared_ptr<storage::folder> folder)
{
	for (const auto& [uuid, subFolder] : folder->subFolders_)
	{
		auto subFolderNode = xmlNode.append_child("folder");
		subFolderNode.append_attribute("name").set_value(subFolder->name_.c_str());
		subFolderNode.append_attribute("uuid").set_value(uuids::to_string(subFolder->uuid_).c_str());

		// Дампим сниппеты папки
		for (const auto& snippet : subFolder->snippets_)
		{
			auto snippetNode = subFolderNode.append_child("snippet");
			dumpSnippet(snippetNode, snippet);
		}

		// Рекурсивно дампим вложенные папки
		dumpFolder(subFolderNode, subFolder);
	}
}
} // namespace data