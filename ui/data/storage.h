#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

#include <uuid.h>

#include "utils/generate_uuid.h"

namespace data
{

class storage
{
public:
	using shared_ptr_t = std::shared_ptr<storage>;

	struct snippet
	{
		std::string title;
		std::string content;
		uuids::uuid uuid;
		bool from_file { false };

		bool operator== (const uuids::uuid& other) const { return uuid == other; }
	};

	using snippet_t = snippet;
	using snippet_shared_ptr_t = std::shared_ptr<snippet_t>;
	using snippets_vec_t = std::vector<snippet_shared_ptr_t>;

	struct folder
	{
		std::string name_;
		std::map<uuids::uuid, std::shared_ptr<folder>> subFolders_;
		snippets_vec_t snippets_;
		std::weak_ptr<folder> parent_;
		uuids::uuid uuid_;

		folder(const std::string& name, uuids::uuid uuid = utils::generate_uuid())
		: name_(name)
		, uuid_(uuid)
		{ }
	};

	using folder_shared_ptr_t = std::shared_ptr<folder>;

	storage();
	folder_shared_ptr_t root() const;
	folder_shared_ptr_t currentFolder() const;
	bool curIsRoot() const;

	void setRoot();

	void folderUp();
	void folderDown(const uuids::uuid& uuid);

	uuids::uuid addFolder(const std::string& name);
	uuids::uuid addSnippet(const std::string title, const std::string& content, bool from_file = false);
	uuids::uuid addSnippet(const uuids::uuid& uuid, const std::string title, const std::string& content, bool from_file = false);

	void deleteFolder(const uuids::uuid& uuid);
	void deleteSnippet(const uuids::uuid& uuid);

	void renameFolder(const uuids::uuid& uuid, const std::string& newName);
	void editSnippet(const uuids::uuid& uuid, const std::string title, const std::string& content, bool from_file = false);

	const folder_shared_ptr_t findFolder(const uuids::uuid& uuid) const;
	const snippet_shared_ptr_t findSnippet(const uuids::uuid& uuid) const;

private:
	folder_shared_ptr_t findFolderImpl(const folder_shared_ptr_t& current, const uuids::uuid& uuid) const;

	folder_shared_ptr_t root_;
	folder_shared_ptr_t currentFolder_;
};

} // namespace data