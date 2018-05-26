import qbs

XCppProduct {

  name: "instrument-selector"
  type: "application"
  files: [
    "g.cpp",
    "g.h",
    "instrumentsmodel.cpp",
    "instrumentsmodel.h",
    "main.cpp",
    "qml.qrc",
    "treemodel.cpp",
    "treemodel.h",
  ]

  Depends { name: "nzmqt" }
  Depends { name: "core" }
  Depends { name: "Qt.quick" }

  cpp.defines: [
    "QT_DEPRECATED_WARNINGS",
    "NZMQT_DEFAULT_ZMQCONTEXT_IMPLEMENTATION=SocketNotifierZMQContext",
    "NZMQT_DEFAULT_IOTHREADS=1",
  ]

}
