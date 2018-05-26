import qbs

XCppProduct {

  type: "staticlibrary"
  name: "core"
  files: [
    "ctlmessage.cpp",
    "ctlmessage.h",
    "exception.cpp",
    "exception.h",
    "exceptions.cpp",
    "exceptions.h",
    "logging.cpp",
    "logging.h",
    "utils.cpp",
    "utils.h",
    "returningdealer.h",
    "returningdealer.cpp",
  ]

  Depends { name: "Qt.core" }
  Depends { name: "Qt.quick" }
  Depends { name: "backward-cpp" }
  Depends { name: "nzmqt" }

  Export {
    Depends { name: "cpp" }
    Depends { name: "Qt.core" }
    Depends { name: "backward-cpp" }
    cpp.includePaths: ["."]
  }

}
