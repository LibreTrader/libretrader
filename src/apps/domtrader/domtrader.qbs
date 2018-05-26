import qbs

XCppProduct {

  name: "domtrader"
  type: "application"
  files: [
    "dommodel.cpp",
    "dommodel.h",
    "g.cpp",
    "g.h",
    "instrumentsmodel.cpp",
    "instrumentsmodel.h",
    "main.cpp",
    "qml.qrc",
    "subscriber.cpp",
    "subscriber.h",
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
