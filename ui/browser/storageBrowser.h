#pragma once

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>

#include "data/storage.h"

namespace ui
{

class InputDialog
{
public:
	InputDialog();
	void Show(const std::string& title, std::function<void(const std::string&)> on_accept, const std::string& default_value = "");
	void Hide();

	bool IsVisible() const { return visible_; }

	ftxui::Component GetComponent() { return component_; }

private:
	bool visible_ = false;
	std::string title_;
	std::string buffer_;
	std::function<void(const std::string&)> callback_;
	ftxui::Component input_;
	ftxui::Component component_;
};

class MultiLineInputDialog
{
public:
	MultiLineInputDialog();
	void Show(const std::string& title, std::function<void(const std::string&, const std::string&, bool)> on_accept, const std::string& default_title = "",
		const std::string& default_content = "", bool default_from_file = false);
	void Hide();

	bool IsVisible() const { return visible_; }

	ftxui::Component GetComponent() { return container_; }

private:
	bool visible_ = false;
	std::string title_;
	std::function<void(const std::string&, const std::string&, bool)> callback_;

	std::string title_buffer_;
	std::string content_buffer_;
	bool from_file_;

	ftxui::Component title_input_;
	ftxui::Component content_input_;
	ftxui::Component checkbox_;
	ftxui::Component container_;
};

class SnippetContentView
{
public:
	SnippetContentView();
	void Show(data::storage::snippet_shared_ptr_t snippet);
	void Hide();

	bool IsVisible() const { return visible_; }

	ftxui::Component GetComponent() { return component_; }

private:
	bool visible_ = false;
	data::storage::snippet_shared_ptr_t snippet_;
	ftxui::Component component_;
};

class StorageTreeView
{
public:
	StorageTreeView(data::storage::shared_ptr_t storage);

	void SetSelectedIndex(int index) { selected_index_ = index; }

	int GetSelectedIndex() const { return selected_index_; }

	ftxui::Component GetComponent() { return component_; }

	// Сигналы для внешних обработчиков
	std::function<void()> on_quit;
	std::function<void()> on_show_snippet;
	std::function<void()> on_add_snippet;
	std::function<void()> on_edit_snippet;
	std::function<void()> on_add_folder;
	std::function<void()> on_delete;
	std::function<void()> on_rename;

private:
	ftxui::Element createKeyHelp();
	std::string getCurrentPath();
	int getItemCount(const data::storage::folder_shared_ptr_t& folder);
	void handleEnter(const data::storage::folder_shared_ptr_t& current_folder);

	data::storage::shared_ptr_t storage_;
	int selected_index_ = 0;
	ftxui::Component component_;
};

class storageBrowser
{
public:
	storageBrowser(data::storage::shared_ptr_t storage, std::function<void()> on_quit);
	ftxui::Component createComponent();

private:
	ftxui::Element render();
	bool handleEvent(ftxui::Event event);

	void handleAddSnippet();
	void handleEditSnippet();
	void handleAddFolder();
	void handleDelete();
	void handleShowSnippet();
	void handleRename();

	data::storage::shared_ptr_t storage_;

	StorageTreeView tree_view_;
	InputDialog input_dialog_;
	MultiLineInputDialog multi_input_dialog_;
	SnippetContentView snippet_view_;
};

void runStorageBrowser(data::storage::shared_ptr_t storage, const std::string& pane);

} // namespace ui