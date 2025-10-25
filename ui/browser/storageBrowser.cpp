#include "browser/storageBrowser.h"
#include <ftxui/component/screen_interactive.hpp>

using namespace ftxui;

namespace ui
{

// InputDialog implementation
InputDialog::InputDialog()
{
	input_ = Input(&buffer_, "Enter text...");

	component_ = Renderer(input_,
		[this]
		{
			if (!visible_)
				return text("");

			return vbox(
							 { text(title_) | bold | center, separator(), input_->Render() | border, separator(), text("Press Enter to confirm, Escape to cancel") | center })
				| border | center;
		});

	component_ |= CatchEvent(
		[this](Event event)
		{
			if (!visible_)
				return false;

			if (event == Event::Return)
			{
				if (callback_ && !buffer_.empty())
				{
					callback_(buffer_);
				}
				Hide();
				return true;
			}
			else if (event == Event::Escape)
			{
				Hide();
				return true;
			}
			return false;
		});
}

void InputDialog::Show(const std::string& title, std::function<void(const std::string&)> on_accept, const std::string& default_value)
{
	title_ = title;
	buffer_ = default_value;
	callback_ = on_accept;
	visible_ = true;
}

void InputDialog::Hide()
{
	visible_ = false;
	buffer_.clear();
	callback_ = nullptr;
}

// MultiLineInputDialog implementation
MultiLineInputDialog::MultiLineInputDialog()
{
	title_input_ = Input(&title_buffer_, "Title...");
	content_input_ = Input(&content_buffer_, "Content...");
	checkbox_ = Checkbox("From File", &from_file_);

	container_ = Container::Vertical({ title_input_, content_input_, checkbox_ });

	container_ = Renderer(container_,
		[this]
		{
			if (!visible_)
				return text("");

			std::vector<Element> content_elements;
			content_elements.push_back(text(title_) | bold | center);
			content_elements.push_back(separator());

			content_elements.push_back(text("Title:") | bold);
			content_elements.push_back(title_input_->Render() | border | flex);
			content_elements.push_back(separator());

			content_elements.push_back(text("Content:") | bold);
			content_elements.push_back(content_input_->Render() | border | flex);
			content_elements.push_back(separator());

			content_elements.push_back(checkbox_->Render());
			content_elements.push_back(separator());

			content_elements.push_back(text("Tab: Switch field, Enter: Confirm, Escape: Cancel") | center);

			return window(text("Input Dialog"), vbox(content_elements)) | size(WIDTH, EQUAL, 100) | center;
		});

	container_ |= CatchEvent(
		[this](Event event)
		{
			if (!visible_)
				return false;

			if (event == Event::Return)
			{
				if (callback_ && !title_buffer_.empty() && !content_buffer_.empty())
				{
					callback_(title_buffer_, content_buffer_, from_file_);
				}
				Hide();
				return true;
			}
			else if (event == Event::Escape)
			{
				Hide();
				return true;
			}
			return false;
		});
}

void MultiLineInputDialog::Show(const std::string& title, std::function<void(const std::string&, const std::string&, bool)> on_accept,
	const std::string& default_title, const std::string& default_content, bool default_from_file)
{
	title_ = title;
	callback_ = on_accept;
	title_buffer_ = default_title;
	content_buffer_ = default_content;
	from_file_ = default_from_file;
	visible_ = true;
}

void MultiLineInputDialog::Hide()
{
	visible_ = false;
	callback_ = nullptr;
	title_buffer_.clear();
	content_buffer_.clear();
	from_file_ = false;
}

// SnippetContentView implementation
SnippetContentView::SnippetContentView()
{
	component_ = Renderer(
		[this]
		{
			if (!visible_ || !snippet_)
				return text("");

			return vbox({ window(text("Snippet: " + snippet_->title),
											vbox({ text("Title: " + snippet_->title) | bold, separator(), text("Content:"), paragraph(snippet_->content) | flex, separator(),
												text("UUID: " + uuids::to_string(snippet_->uuid)), text("From file: " + std::string(snippet_->from_file ? "Yes" : "No")) })
												| flex | frame),
							 text("Press any key to return") | center })
				| flex;
		});

	component_ |= CatchEvent(
		[this](Event event)
		{
			if (!visible_)
				return false;

			if (event.is_character() || event.is_mouse())
			{
				Hide();
				return true;
			}
			return false;
		});
}

void SnippetContentView::Show(data::storage::snippet_shared_ptr_t snippet)
{
	snippet_ = snippet;
	visible_ = true;
}

void SnippetContentView::Hide()
{
	visible_ = false;
	snippet_ = nullptr;
}

// StorageTreeView implementation
StorageTreeView::StorageTreeView(data::storage::shared_ptr_t storage)
: storage_(storage)
{
	component_ = Renderer(
		[this]
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
				auto snippet_text = snippet->title + (snippet->from_file ? " [FILE]" : "");
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

			return window(text("Storage Browser"), vbox({ hbox(instructions) | flex, separator(), list | flex }) | flex);
		});

	component_ |= CatchEvent(
		[this](Event event)
		{
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
				if (on_delete)
					on_delete();
				return true;
			}
			else if (event == Event::Character('a'))
			{
				if (on_add_snippet)
					on_add_snippet();
				return true;
			}
			else if (event == Event::Character('e'))
			{
				if (on_edit_snippet)
					on_edit_snippet();
				return true;
			}
			else if (event == Event::Character('n'))
			{
				if (on_add_folder)
					on_add_folder();
				return true;
			}
			else if (event == Event::Character('s'))
			{
				if (on_show_snippet)
					on_show_snippet();
				return true;
			}
			else if (event == Event::Character('q'))
			{
				if (on_quit)
					on_quit();
				return true;
			}

			return false;
		});
}

std::string StorageTreeView::getCurrentPath()
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

int StorageTreeView::getItemCount(const data::storage::folder_shared_ptr_t& folder)
{
	int count = folder->subFolders_.size() + folder->snippets_.size();
	if (!storage_->curIsRoot())
	{
		count++;
	}
	return count;
}

void StorageTreeView::handleEnter(const data::storage::folder_shared_ptr_t& current_folder)
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

// storageBrowser implementation
storageBrowser::storageBrowser(data::storage::shared_ptr_t storage)
: storage_(storage)
, tree_view_(storage)
{
	// Настраиваем обработчики для tree view
	tree_view_.on_quit = []() { /* handled by screen */ };
	tree_view_.on_show_snippet = [this]()
	{
		handleShowSnippet();
	};
	tree_view_.on_add_snippet = [this]()
	{
		handleAddSnippet();
	};
	tree_view_.on_edit_snippet = [this]()
	{
		handleEditSnippet();
	};
	tree_view_.on_add_folder = [this]()
	{
		handleAddFolder();
	};
	tree_view_.on_delete = [this]()
	{
		handleDelete();
	};
}

Component storageBrowser::createComponent()
{
	auto component = Renderer([this] { return this->render(); });

	return CatchEvent(component, [this](Event event) { return this->handleEvent(event); });
}

Element storageBrowser::render()
{
	if (snippet_view_.IsVisible())
	{
		return snippet_view_.GetComponent()->Render();
	}

	auto tree_element = tree_view_.GetComponent()->Render();

	if (input_dialog_.IsVisible())
	{
		return dbox({ tree_element, input_dialog_.GetComponent()->Render() });
	}
	else if (multi_input_dialog_.IsVisible())
	{
		return dbox({ tree_element, multi_input_dialog_.GetComponent()->Render() });
	}

	return tree_element;
}

bool storageBrowser::handleEvent(Event event)
{
	if (snippet_view_.IsVisible())
	{
		return snippet_view_.GetComponent()->OnEvent(event);
	}

	if (input_dialog_.IsVisible())
	{
		return input_dialog_.GetComponent()->OnEvent(event);
	}

	if (multi_input_dialog_.IsVisible())
	{
		return multi_input_dialog_.GetComponent()->OnEvent(event);
	}

	return tree_view_.GetComponent()->OnEvent(event);
}

void storageBrowser::handleAddSnippet()
{
	multi_input_dialog_.Show("Add New Snippet",
		[this](const std::string& title, const std::string& content, bool from_file)
		{
			if (!title.empty() && !content.empty())
			{
				storage_->addSnippet(title, content, from_file);
			}
		});
}

void storageBrowser::handleEditSnippet()
{
	auto current_folder = storage_->currentFolder();
	int adjusted_index = tree_view_.GetSelectedIndex();

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
			multi_input_dialog_.Show(
				"Edit Snippet",
				[this, snippet](const std::string& title, const std::string& content, bool from_file)
				{
					if (!title.empty() && !content.empty())
					{
						storage_->editSnippet(snippet->uuid, title, content, from_file);
					}
				},
				snippet->title, snippet->content, snippet->from_file);
		}
	}
}

void storageBrowser::handleAddFolder()
{
	input_dialog_.Show("Add New Folder",
		[this](const std::string& name)
		{
			if (!name.empty())
			{
				storage_->addFolder(name);
			}
		});
}

void storageBrowser::handleDelete()
{
	auto current_folder = storage_->currentFolder();
	int adjusted_index = tree_view_.GetSelectedIndex();

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
}

void storageBrowser::handleShowSnippet()
{
	auto current_folder = storage_->currentFolder();
	int adjusted_index = tree_view_.GetSelectedIndex();

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
			snippet_view_.Show(current_folder->snippets_[snippet_index]);
		}
	}
}

void runStorageBrowser(data::storage::shared_ptr_t storage)
{
	auto screen = ScreenInteractive::TerminalOutput();
	storageBrowser browser(storage);
	auto component = browser.createComponent();

	screen.Loop(component);
}

} // namespace ui