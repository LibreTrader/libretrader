import qbs

Product {

  name: "backward-cpp"
  type: "headers"

  Group {
    name: "Headers"
    files: ["backward.hpp"]
    fileTags: ["hpp"]
  }

  Export {

    Depends { name: "cpp" }
    cpp.includePaths: ["."]

    Properties {
      condition: qbs.targetOS.contains("linux")
      cpp.defines: [
        "BACKWARD_HAS_BFD=1",
        "BACKWARD_HAS_DW=1",
      ]
      cpp.dynamicLibraries: [
        "dw",
        "dl",
        "bfd",
      ]
    }

  }
}
