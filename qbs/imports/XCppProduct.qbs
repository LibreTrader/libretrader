import qbs

XProduct {

  Depends { name: "cpp" }

  cpp.minimumMacosVersion: project.minimumMacosVersion

  cpp.rpaths: ["$ORIGIN/../lib"]

  cpp.libraryPaths: {
    if (platform === "linux")
      return [project.libRoot + "/linux"]
    if (platform === "windows")
      return [project.libRoot + "/windows"]
    if (platform === "macos")
      return [project.libRoot + "/macos"]
    return []
  }

  Group {
    fileTagsFilter: "application"
    qbs.install: true
    qbs.installDir: "bin"
  }

}
