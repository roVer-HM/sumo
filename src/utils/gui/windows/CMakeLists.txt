set(utils_gui_windows_STAT_SRCS
   GUIAppEnum.h
   GUIDanielPerspectiveChanger.cpp
   GUIDanielPerspectiveChanger.h
   GUIDialog_EditViewport.cpp
   GUIDialog_EditViewport.h
   GUIDialog_ViewSettings.cpp
   GUIDialog_ViewSettings.h
   GUIGlChildWindow.cpp
   GUIGlChildWindow.h
   GUIMainWindow.cpp
   GUIMainWindow.h
   GUIPerspectiveChanger.cpp
   GUIPerspectiveChanger.h
   GUISUMOAbstractView.cpp
   GUISUMOAbstractView.h
   GUIDialog_ChooserAbstract.cpp
   GUIDialog_ChooserAbstract.h
)

add_library(utils_gui_windows STATIC ${utils_gui_windows_STAT_SRCS})
set_property(TARGET utils_gui_windows PROPERTY PROJECT_LABEL "z_utils_gui_windows")
