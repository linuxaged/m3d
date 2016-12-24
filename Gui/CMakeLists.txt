add_library(Gui SHARED
	src/GUIButton.cpp
	src/GUICheckBox.cpp
	src/GUIController.cpp
	src/GUIElement.cpp
	src/GUIImage.cpp
	src/GUIPanel.cpp
	src/GUITextCaption.cpp)

set_target_properties(Gui PROPERTIES FOLDER "common")

target_include_directories(Gui PUBLIC ./include)
target_include_directories(Gui PRIVATE ./src)

target_link_libraries(Gui Math)