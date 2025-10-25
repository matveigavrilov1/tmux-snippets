#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>

#include "data/storage.h"

namespace ui
{

class storageBrowser
{
public:
	storageBrowser(data::storage::shared_ptr_t storage);
	ftxui::Component createComponent();

private:
	ftxui::Element renderTree();
	ftxui::Element renderSnippetContent();
	std::string getCurrentPath();
	bool handleEvent(ftxui::Event event);
	int getItemCount(const data::storage::folder_shared_ptr_t& folder);
	void handleEnter(const data::storage::folder_shared_ptr_t& current_folder);
	void handleDelete(const data::storage::folder_shared_ptr_t& current_folder);
	void handleAddSnippet(const data::storage::folder_shared_ptr_t& current_folder);
	void handleEditSnippet(const data::storage::folder_shared_ptr_t& current_folder);
	void handleAddFolder(const data::storage::folder_shared_ptr_t& current_folder);

	void showInputDialog(const std::string& title, std::function<void(const std::string&)> on_accept, const std::string& default_value = "");
	void showMultiLineInputDialog(const std::string& title, std::function<void(const std::string&, const std::string&)> on_accept,
		const std::string& default_title = "", const std::string& default_content = "");

	data::storage::shared_ptr_t storage_;
	int selected_index_;

	enum class Mode
	{
		Browsing,
		InputDialog,
		MultiLineInputDialog
	};
	Mode current_mode_ = Mode::Browsing;

	std::string input_title_;
	std::string input_buffer_;
	std::string input_buffer2_;
	std::function<void(const std::string&)> input_callback_;
	std::function<void(const std::string&, const std::string&)> multi_input_callback_;

	data::storage::snippet_shared_ptr_t selected_snippet_ = nullptr;
	bool show_snippet_content_ = false;
};

// Функции для запуска браузеров
void runStoragwBrowser(data::storage::shared_ptr_t storage);

} // namespace ui