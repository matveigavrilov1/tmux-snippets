#include "data/storage.h"
#include "utils/generate_uuid.h"

#include <algorithm>
#include <memory>

namespace data
{

storage::storage()
{
	root_ = std::make_shared<folder>("/");
	currentFolder_ = root_;
}

std::shared_ptr<storage::folder> storage::root() const
{
	return root_;
}

std::shared_ptr<storage::folder> storage::currentFolder() const
{
	return currentFolder_;
}

bool storage::curIsRoot() const
{
	return root_.get() == currentFolder_.get();
}

void storage::setRoot()
{
	currentFolder_ = root_;
}

void storage::folderUp()
{
	if (currentFolder_ != root_ && !currentFolder_->parent_.expired())
	{
		currentFolder_ = currentFolder_->parent_.lock();
	}
}

void storage::folderDown(const uuids::uuid& folder_uuid)
{
	auto it = std::find_if(
		currentFolder_->subFolders_.begin(), currentFolder_->subFolders_.end(), [&folder_uuid](const auto& pair) { return pair.second->uuid_ == folder_uuid; });

	if (it != currentFolder_->subFolders_.end())
	{
		currentFolder_ = it->second;
	}
}

uuids::uuid storage::addFolder(const std::string& name)
{
	auto newFolder = std::make_shared<folder>(name);
	newFolder->parent_ = currentFolder_;
	currentFolder_->subFolders_[newFolder->uuid_] = newFolder;
	return newFolder->uuid_;
}

uuids::uuid storage::addSnippet(const std::string title, const std::string& content, bool from_file)
{
	auto newSnippet = std::make_shared<snippet_t>(title, content, utils::generate_uuid(), from_file);
	currentFolder_->snippets_.push_back(newSnippet);
	return newSnippet->uuid;
}

uuids::uuid storage::addSnippet(const uuids::uuid& uuid, const std::string title, const std::string& content, bool from_file)
{
	auto newSnippet = std::make_shared<snippet_t>(title, content, uuid, from_file);
	currentFolder_->snippets_.push_back(newSnippet);
	return newSnippet->uuid;
}

void storage::deleteFolder(const uuids::uuid& uuid)
{
	auto it = currentFolder_->subFolders_.find(uuid);
	if (it != currentFolder_->subFolders_.end())
	{
		currentFolder_->subFolders_.erase(it);
	}
}

void storage::deleteSnippet(const uuids::uuid& uuid)
{
	auto& commands = currentFolder_->snippets_;
	commands.erase(std::remove_if(commands.begin(), commands.end(), [&uuid](const auto& cmd) { return cmd->uuid == uuid; }), commands.end());
}

void storage::renameFolder(const uuids::uuid& folder_uuid, const std::string& newName)
{
	auto it = currentFolder_->subFolders_.find(folder_uuid);
	if (it != currentFolder_->subFolders_.end())
	{
		it->second->name_ = newName;
	}
}

void storage::editSnippet(const uuids::uuid& uuid, const std::string title, const std::string& content, bool from_file)
{
	auto& commands = currentFolder_->snippets_;
	auto it = std::find_if(commands.begin(), commands.end(), [&uuid](const auto& cmd) { return cmd->uuid == uuid; });

	if (it != commands.end())
	{
		(*it)->content = content;
		(*it)->title = title;
		(*it)->from_file = from_file;
	}
}

const storage::folder_shared_ptr_t storage::findFolder(const uuids::uuid& uuid) const
{
	return findFolderImpl(root_, uuid);
}

const storage::snippet_shared_ptr_t storage::findSnippet(const uuids::uuid& uuid) const
{
	// Search in current folder first
	auto it = std::find_if(currentFolder_->snippets_.begin(), currentFolder_->snippets_.end(), [&uuid](const auto& cmd) { return cmd->uuid == uuid; });

	if (it != currentFolder_->snippets_.end())
	{
		return (*it);
	}

	// If not found, search recursively in all folders
	folder_shared_ptr_t folder_with_command;
	std::function<bool(const folder_shared_ptr_t&)> search = [&](const folder_shared_ptr_t& f)
	{
		auto cmd_it = std::find_if(f->snippets_.begin(), f->snippets_.end(), [&uuid](const auto& cmd) { return cmd->uuid == uuid; });

		if (cmd_it != f->snippets_.end())
		{
			folder_with_command = f;
			return true;
		}

		for (const auto& [_, subfolder] : f->subFolders_)
		{
			if (search(subfolder))
				return true;
		}

		return false;
	};

	if (search(root_))
	{
		auto cmd_it =
			std::find_if(folder_with_command->snippets_.begin(), folder_with_command->snippets_.end(), [&uuid](const auto& cmd) { return cmd->uuid == uuid; });
		if (cmd_it != folder_with_command->snippets_.end())
		{
			return (*cmd_it);
		}
	}

	return nullptr;
}

storage::folder_shared_ptr_t storage::findFolderImpl(const folder_shared_ptr_t& current, const uuids::uuid& uuid) const
{
	if (current->uuid_ == uuid)
	{
		return current;
	}

	for (const auto& [_, subfolder] : current->subFolders_)
	{
		auto found = findFolderImpl(subfolder, uuid);
		if (found)
		{
			return found;
		}
	}

	return nullptr;
}

} // namespace data