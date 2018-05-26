import qbs

XProduct {

  type: "copy"
  name: "lib"

  Group {
    condition: platform == "linux"
    name: "Shared Libraries (Linux)"
    files: "linux/*"
    qbs.install: true
    qbs.installDir: "lib"
  }

  Group {
    condition: platform == "windows"
    name: "Shared Libraries (Windows)"
    files: "windows/*.dll"
    qbs.install: true
    qbs.installDir: "bin"
  }

  Group {
    condition: platform == "macos"
    name: "Shared Libraries (MacOS)"
    files: "macos/*"
    qbs.install: true
    qbs.installDir: "lib"
  }

}
