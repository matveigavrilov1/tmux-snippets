cmake_minimum_required(VERSION 3.10.0)

set(UI_TARGET_SOURCES main.cpp
	browser/storageBrowser.cpp
	data/storage.cpp
	utils/generate_uuid.cpp
	utils/send_to_tmux.cpp
)

add_executable(${UI_EXECUTABLE} ${UI_TARGET_SOURCES})

target_include_directories(${UI_EXECUTABLE}
	PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${UI_EXECUTABLE}
	ftxui::ftxui
	stduuid::stduuid
	pugixml::pugixml
)