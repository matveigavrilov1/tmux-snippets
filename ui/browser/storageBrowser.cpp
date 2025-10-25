#include "browser/storageBrowser.h"

#include <ftxui/component/screen_interactive.hpp>

using namespace ftxui;

namespace ui
{


storageBrowser::storageBrowser(data::storage::shared_ptr_t storage)
: storage_(storage)
, selected_index_(0)
, current_mode_(Mode::Browsing)
{ }

Component storageBrowser::createComponent()
{
	auto tree_component = Container::Vertical({});


	auto renderer = Renderer(tree_component,
		[this]
		{
			if (show_snippet_content_ && selected_snippet_)
			{
				return renderSnippetContent();
			}
			return renderTree();
		});


	auto component = CatchEvent(renderer, [this](Event event) { return this->handleEvent(event); });

	return component;
}

Element storageBrowser::renderTree()
{
	auto current_folder = storage_->currentFolder();


	std::vector<Element> elements;


	elements.push_back(text("Current: " + getCurrentPath()) | bold);
	elements.push_back(separator());


	if (!storage_->curIsRoot())
	{
		auto parent_text = "/..";
		bool is_selected = (selected_index_ == 0);
		auto element = text(parent_text);
		if (is_selected)
		{
			element = element | inverted;
		}
		elements.push_back(element);
	}


	int folder_index = storage_->curIsRoot() ? 0 : 1;
	for (const auto& [uuid, folder] : current_folder->subFolders_)
	{
		auto folder_text = "/" + folder->name_;
		bool is_selected = (selected_index_ == folder_index);
		auto element = text(folder_text);
		if (is_selected)
		{
			element = element | inverted;
		}
		elements.push_back(element);
		folder_index++;
	}


	for (const auto& snippet : current_folder->snippets_)
	{
		auto snippet_text = snippet->title;
		bool is_selected = (selected_index_ == folder_index);
		auto element = text(snippet_text);
		if (is_selected)
		{
			element = element | inverted;
		}
		elements.push_back(element);
		folder_index++;
	}


	auto list = vbox(elements);


	std::vector<Element> instructions = { text("Navigation: ↑↓ Move, Enter Open, Backspace Back"),
		text("Actions: 'a' Add snippet, 'd' Delete, 'e' Edit, 'n' New folder"), text("'s' Show snippet content, 'q' Quit") };


	Element dialog_element = text("");
	if (current_mode_ == Mode::InputDialog)
	{
		dialog_element =
			vbox({ text(input_title_) | bold, separator(), hbox({ text("> "), text(input_buffer_) }) | border, text("Press Enter to confirm, Escape to cancel") })
			| border | center;
	}
	else if (current_mode_ == Mode::MultiLineInputDialog)
	{
		dialog_element = vbox({ text(input_title_) | bold, separator(), hbox({ text("Title: "), text(input_buffer_) }) | border,
											 hbox({ text("Content: "), text(input_buffer2_) }) | border, text("Press Enter to confirm, Escape to cancel") })
			| border | center;
	}


	return vbox({ window(text("Storage Browser"), vbox({ hbox(instructions) | flex, separator(), list | flex }) | flex), dialog_element }) | flex;
}

Element storageBrowser::renderSnippetContent()
{
	if (!selected_snippet_)
	{
		return text("No snippet selected");
	}

	return vbox({ window(text("Snippet: " + selected_snippet_->title),
									vbox({ text("Title: " + selected_snippet_->title) | bold, separator(), text("Content:"), paragraph(selected_snippet_->content) | flex,
										separator(), text("UUID: " + uuids::to_string(selected_snippet_->uuid)),
										text("From file: " + std::string(selected_snippet_->from_file ? "Yes" : "No")) })
										| flex | frame),
					 text("Press any key to return") | center })
		| flex;
}

std::string storageBrowser::getCurrentPath()
{
	auto current = storage_->currentFolder();
	std::vector<std::string> path_parts;


	auto folder = current;
	while (folder && folder != storage_->root())
	{
		path_parts.push_back(folder->name_);
		if (auto parent = folder->parent_.lock())
		{
			folder = parent;
		}
		else
		{
			break;
		}
	}

	path_parts.push_back("Root");
	std::reverse(path_parts.begin(), path_parts.end());

	std::string path;
	for (size_t i = 0; i < path_parts.size(); ++i)
	{
		if (i > 0)
			path += " / ";
		path += path_parts[i];
	}

	return path;
}

bool storageBrowser::handleEvent(Event event)
{
	if (show_snippet_content_)
	{
		if (event.is_character() || event.is_mouse())
		{
			show_snippet_content_ = false;
			selected_snippet_ = nullptr;
		}
		return true;
	}


	if (current_mode_ == Mode::InputDialog)
	{
		if (event == Event::Return)
		{
			current_mode_ = Mode::Browsing;
			if (input_callback_)
			{
				input_callback_(input_buffer_);
			}
			input_buffer_.clear();
			return true;
		}
		else if (event == Event::Escape)
		{
			current_mode_ = Mode::Browsing;
			input_buffer_.clear();
			return true;
		}
		else if (event.is_character())
		{
			input_buffer_ += event.character();
			return true;
		}
		else if (event == Event::Backspace)
		{
			if (!input_buffer_.empty())
			{
				input_buffer_.pop_back();
			}
			return true;
		}
		return false;
	}
	else if (current_mode_ == Mode::MultiLineInputDialog)
	{
		if (event == Event::Return)
		{
			current_mode_ = Mode::Browsing;
			if (multi_input_callback_)
			{
				multi_input_callback_(input_buffer_, input_buffer2_);
			}
			input_buffer_.clear();
			input_buffer2_.clear();
			return true;
		}
		else if (event == Event::Escape)
		{
			current_mode_ = Mode::Browsing;
			input_buffer_.clear();
			input_buffer2_.clear();
			return true;
		}
		else if (event.is_character())
		{
			input_buffer_ += event.character();
			return true;
		}
		else if (event == Event::Backspace)
		{
			if (!input_buffer_.empty())
			{
				input_buffer_.pop_back();
			}
			return true;
		}
		return false;
	}


	auto current_folder = storage_->currentFolder();
	int item_count = getItemCount(current_folder);

	if (event == Event::ArrowUp)
	{
		selected_index_ = std::max(0, selected_index_ - 1);
		return true;
	}
	else if (event == Event::ArrowDown)
	{
		selected_index_ = std::min(item_count - 1, selected_index_ + 1);
		return true;
	}
	else if (event == Event::Return)
	{
		handleEnter(current_folder);
		return true;
	}
	else if (event == Event::Backspace)
	{
		if (!storage_->curIsRoot())
		{
			storage_->folderUp();
			selected_index_ = 0;
		}
		return true;
	}
	else if (event == Event::Character('d') || event == Event::Delete)
	{
		handleDelete(current_folder);
		return true;
	}
	else if (event == Event::Character('a'))
	{
		handleAddSnippet(current_folder);
		return true;
	}
	else if (event == Event::Character('e'))
	{
		handleEditSnippet(current_folder);
		return true;
	}
	else if (event == Event::Character('n'))
	{
		handleAddFolder(current_folder);
		return true;
	}
	else if (event == Event::Character('s'))
	{
		int adjusted_index = selected_index_;
		if (!storage_->curIsRoot())
		{
			if (adjusted_index == 0)
				return true;
			adjusted_index--;
		}

		if (adjusted_index >= current_folder->subFolders_.size())
		{
			int snippet_index = adjusted_index - current_folder->subFolders_.size();
			if (snippet_index < current_folder->snippets_.size())
			{
				selected_snippet_ = current_folder->snippets_[snippet_index];
				show_snippet_content_ = true;
			}
		}
		return true;
	}
	else if (event == Event::Character('q'))
	{
		return false;
	}

	return false;
}

int storageBrowser::getItemCount(const data::storage::folder_shared_ptr_t& folder)
{
	int count = folder->subFolders_.size() + folder->snippets_.size();
	if (!storage_->curIsRoot())
	{
		count++;
	}
	return count;
}

void storageBrowser::handleEnter(const data::storage::folder_shared_ptr_t& current_folder)
{
	int adjusted_index = selected_index_;


	if (!storage_->curIsRoot())
	{
		if (adjusted_index == 0)
		{
			storage_->folderUp();
			selected_index_ = 0;
			return;
		}
		adjusted_index--;
	}


	if (adjusted_index < current_folder->subFolders_.size())
	{
		auto it = current_folder->subFolders_.begin();
		std::advance(it, adjusted_index);
		storage_->folderDown(it->first);
		selected_index_ = 0;
	}
}

void storageBrowser::handleDelete(const data::storage::folder_shared_ptr_t& current_folder)
{
	int adjusted_index = selected_index_;

	if (!storage_->curIsRoot())
	{
		if (adjusted_index == 0)
			return;
		adjusted_index--;
	}

	if (adjusted_index < current_folder->subFolders_.size())
	{
		auto it = current_folder->subFolders_.begin();
		std::advance(it, adjusted_index);
		storage_->deleteFolder(it->first);
	}
	else
	{
		int snippet_index = adjusted_index - current_folder->subFolders_.size();
		if (snippet_index < current_folder->snippets_.size())
		{
			auto snippet = current_folder->snippets_[snippet_index];
			storage_->deleteSnippet(snippet->uuid);
		}
	}


	if (selected_index_ >= getItemCount(current_folder))
	{
		selected_index_ = std::max(0, getItemCount(current_folder) - 1);
	}
}

void storageBrowser::handleAddSnippet(const data::storage::folder_shared_ptr_t& current_folder)
{
	showMultiLineInputDialog("Add New Snippet",
		[this, current_folder](const std::string& title, const std::string& content)
		{
			if (!title.empty() && !content.empty())
			{
				storage_->addSnippet(title, content);
			}
		});
}

void storageBrowser::handleEditSnippet(const data::storage::folder_shared_ptr_t& current_folder)
{
	int adjusted_index = selected_index_;

	if (!storage_->curIsRoot())
	{
		if (adjusted_index == 0)
			return;
		adjusted_index--;
	}


	if (adjusted_index >= current_folder->subFolders_.size())
	{
		int snippet_index = adjusted_index - current_folder->subFolders_.size();
		if (snippet_index < current_folder->snippets_.size())
		{
			auto snippet = current_folder->snippets_[snippet_index];

			showMultiLineInputDialog(
				"Edit Snippet",
				[this, snippet](const std::string& title, const std::string& content)
				{
					if (!title.empty() && !content.empty())
					{
						storage_->editSnippet(snippet->uuid, title, content, snippet->from_file);
					}
				},
				snippet->title, snippet->content);
		}
	}
}

void storageBrowser::handleAddFolder(const data::storage::folder_shared_ptr_t& current_folder)
{
	showInputDialog("Add New Folder",
		[this](const std::string& name)
		{
			if (!name.empty())
			{
				storage_->addFolder(name);
			}
		});
}

void storageBrowser::showInputDialog(const std::string& title, std::function<void(const std::string&)> on_accept, const std::string& default_value)
{
	input_title_ = title;
	input_buffer_ = default_value;
	input_callback_ = on_accept;
	current_mode_ = Mode::InputDialog;
}

void storageBrowser::showMultiLineInputDialog(const std::string& title, std::function<void(const std::string&, const std::string&)> on_accept,
	const std::string& default_title, const std::string& default_content)
{
	input_title_ = title;
	input_buffer_ = default_title;
	input_buffer2_ = default_content;
	multi_input_callback_ = on_accept;
	current_mode_ = Mode::MultiLineInputDialog;
}

void runStoragwBrowser(data::storage::shared_ptr_t storage)
{
	auto screen = ScreenInteractive::TerminalOutput();
	storageBrowser browser(storage);
	auto component = browser.createComponent();

	screen.Loop(component);
}

} // namespace ui
