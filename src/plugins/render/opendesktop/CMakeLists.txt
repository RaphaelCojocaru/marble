PROJECT( OpenDesktopPlugin )

INCLUDE_DIRECTORIES(
 ${CMAKE_CURRENT_SOURCE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR}
 ${QT_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(${Qt5WebKitWidgets_INCLUDE_DIRS})

set( my_SRCS
  OpenDesktopPlugin.cpp
  OpenDesktopItem.cpp
  OpenDesktopModel.cpp
)

set( opendesktop_UI OpenDesktopConfigWidget.ui )
qt_wrap_ui( my_SRCS ${opendesktop_UI} )

marble_add_plugin( OpenDesktopPlugin ${my_SRCS} )
