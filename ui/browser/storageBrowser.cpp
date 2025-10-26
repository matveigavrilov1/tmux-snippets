#include "browser/storageBrowser.h"

#include <ftxui/component/screen_interactive.hpp>

#include "utils/exePathManager.h"
#include "utils/send_to_tmux.h"

#include <fstream>

using namespace ftxui;

namespace ui
{
static std::string paneToSendCommand;

// InputDialog implementation
InputDialog::InputDialog()
{
	input_ = Input(&buffer_, "Enter text...");

	component_ = Renderer(input_,
		[this]
		{
			if (!visible_)
				return text("");

			return vbox({ text(title_) | bold | center, separator(), hbox({ text("> "), input_->Render() }) | border, separator(),
							 hbox({ text("[Enter]") | bold, text(" Confirm  "), text("[Esc]") | bold, text(" Cancel") }) | center })
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

			content_elements.push_back(
				hbox({ text("[Enter]") | bold, text(" Confirm  "), text("[Esc]") | bold, text(" Cancel  "), text("[Tab]") | bold, text(" Switch field") }) | center);

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

			auto list = vbox(std::move(elements));

			return window(text("Snippets"), vbox({ createKeyHelp(), separator(), list | flex }) | flex);
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
			else if (event == Event::Delete)
			{
				if (on_delete)
					on_delete();
				return true;
			}
			else if (event == Event::F1)
			{
				if (on_add_snippet)
					on_add_snippet();
				return true;
			}
			else if (event == Event::F2)
			{
				if (on_edit_item)
					on_edit_item();
				return true;
			}
			else if (event == Event::F3)
			{
				if (on_add_folder)
					on_add_folder();
				return true;
			}
			else if (event == Event::F4)
			{
				if (on_show_snippet)
					on_show_snippet();
				return true;
			}
			else if (event == Event::Escape)
			{
				if (on_quit)
					on_quit();
				return true;
			}

			return false;
		});
}

Element StorageTreeView::createKeyHelp()
{
	return hbox({ text("[F1] Add "), text("[F2] Edit "), text("[F3] Add Folder "), text("[F4] View "), text("[Del] Delete "), text("[Esc] Quit") }) | bold;
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

	path_parts.push_back("/");
	std::reverse(path_parts.begin(), path_parts.end());

	std::string path;
	for (size_t i = 0; i < path_parts.size(); ++i)
	{
		path += path_parts[i];
		if (i > 0)
			path += "/";
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
		return;
	}

	if (adjusted_index >= current_folder->subFolders_.size())
	{
		int snippet_index = adjusted_index - current_folder->subFolders_.size();
		if (snippet_index < current_folder->snippets_.size())
		{
			auto snippet = current_folder->snippets_[snippet_index];
			std::string content_to_send;

			if (snippet->from_file)
			{
				std::filesystem::path file_path = utils::exePathManager::getInstance().getFileSnippetPath(snippet->content);
				std::ifstream file(file_path);
				if (file.is_open())
				{
					std::stringstream buffer;
					buffer << file.rdbuf();
					content_to_send = buffer.str();
					file.close();
				}
			}
			else
			{
				// Используем содержимое напрямую
				content_to_send = snippet->content;
			}

			utils::sendCommandToTmux(content_to_send, paneToSendCommand);

			if (on_quit)
				on_quit();
			return;
		}
	}

	return;
}

storageBrowser::storageBrowser(data::storage::shared_ptr_t storage, std::function<void()> on_quit)
: storage_(storage)
, tree_view_(storage)
{
	tree_view_.on_show_snippet = [this]()
	{
		handleShowSnippet();
	};
	tree_view_.on_edit_item = [this]()
	{
		handleEditItem();
	};
	tree_view_.on_add_snippet = [this]()
	{
		handleAddSnippet();
	};
	tree_view_.on_add_folder = [this]()
	{
		handleAddFolder();
	};
	tree_view_.on_delete = [this]()
	{
		handleDelete();
	};
	tree_view_.on_quit = on_quit;
}

Component storageBrowser::createComponent()
{
	auto component = Renderer([this] { return this->render(); });

	return CatchEvent(component, [this](Event event) { return this->handleEvent(event); });
}

Element storageBrowser::render()
{
	// Если открыт просмотр сниппета - показываем только его
	if (snippet_view_.IsVisible())
	{
		return snippet_view_.GetComponent()->Render();
	}

	// Если открыт диалог редактирования - показываем только его (без основного окна)
	if (multi_input_dialog_.IsVisible())
	{
		return multi_input_dialog_.GetComponent()->Render();
	}

	// Если открыт простой диалог - показываем его поверх основного окна
	if (input_dialog_.IsVisible())
	{
		return dbox({ tree_view_.GetComponent()->Render(), input_dialog_.GetComponent()->Render() });
	}

	// Иначе показываем только основное окно
	return tree_view_.GetComponent()->Render();
}

bool storageBrowser::handleEvent(Event event)
{
	// Приоритет обработки событий: сниппет -> диалоги -> основное окно
	if (snippet_view_.IsVisible())
	{
		return snippet_view_.GetComponent()->OnEvent(event);
	}

	if (multi_input_dialog_.IsVisible())
	{
		return multi_input_dialog_.GetComponent()->OnEvent(event);
	}

	if (input_dialog_.IsVisible())
	{
		return input_dialog_.GetComponent()->OnEvent(event);
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

void storageBrowser::handleEditItem()
{
	auto current_folder = storage_->currentFolder();
	int adjusted_index = tree_view_.GetSelectedIndex();

	if (!storage_->curIsRoot())
	{
		if (adjusted_index == 0)
			return;
		adjusted_index--;
	}

	// Если выбран сниппет - открываем многострочное редактирование
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
	// Если выбрана папка - открываем простое переименование
	else if (adjusted_index < current_folder->subFolders_.size())
	{
		auto it = current_folder->subFolders_.begin();
		std::advance(it, adjusted_index);
		auto folder = it->second;

		input_dialog_.Show(
			"Rename Folder",
			[this, folder](const std::string& new_name)
			{
				if (!new_name.empty())
				{
					storage_->renameFolder(folder->uuid_, new_name);
				}
			},
			folder->name_);
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

void runStorageBrowser(data::storage::shared_ptr_t storage, const std::string& pane)
{
	paneToSendCommand = pane;
	auto screen = ScreenInteractive::TerminalOutput();
	storageBrowser browser(storage, [&screen]() { screen.Exit(); });
	auto component = browser.createComponent();
	screen.Loop(component);
}

} // namespace ui