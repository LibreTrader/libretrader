import qbs

XCppProduct {

  name: "launcher"
  type: "application"
  files: [
    "applauncher.cpp",
    "applauncher.h",
    "g.cpp",
    "g.h",
    "main.cpp",
    "messagebroker.cpp",
    "messagebroker.h",
    "qml.qrc",
  ]

  Depends { name: "nzmqt" }
  Depends { name: "core" }
  Depends { name: "Qt.quick" }

  cpp.defines: [
    "QT_DEPRECATED_WARNINGS",
    "NZMQT_DEFAULT_ZMQCONTEXT_IMPLEMENTATION=SocketNotifierZMQContext",
    "NZMQT_DEFAULT_IOTHREADS=1",
  ]

  //    Additional import path used to resolve QML modules in Qt Creator's code model
  //    property pathList qmlImportPaths: []

}
