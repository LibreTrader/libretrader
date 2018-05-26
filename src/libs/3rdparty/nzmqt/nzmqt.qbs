import qbs

XCppProduct {

  id: root
  name: "nzmqt"
  type: "staticlibrary"

  Group {
    name: "nzmqt headers"
    files: ["include/nzmqt/*.hpp"]
    fileTags: ["hpp"]
  }

  Group {
    name: "cppzmq headers"
    files: ["3rdparty/cppzmq/*.hpp"]
    fileTags: ["hpp"]
  }

  Group {
    name: "libzmq headers"
    files: ["3rdparty/libzmq/*.h"]
    fileTags: ["hpp"]
  }

  Group {
    name: "cpp files"
    files: ["src/nzmqt/nzmqt.cpp"]
  }

  Depends { name: "cpp" }
  Depends { name: "Qt.core" }

  cpp.defines: [
    "NZMQT_LIB",
    // "NZMQT_SHARED_LIB",
    "NZMQT_STATIC_LIB",
  ]

  cpp.dynamicLibraries: [
    "zmq"
  ]

  cpp.includePaths: [
    "./include",
    "./3rdparty/cppzmq",
    "./3rdparty/libzmq"
  ]

  Export {
    Depends { name: "cpp" }
    cpp.includePaths: [
      "./include",
      "./3rdparty/cppzmq",
      "./3rdparty/libzmq"
    ]
    cpp.dynamicLibraries: [
      "zmq"
    ]
    cpp.defines: [
      "NZMQT_LIB",
      "NZMQT_STATIC_LIB",
    ]

  }
}
