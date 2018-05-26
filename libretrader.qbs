import qbs

Project {

  name: "LibreTrader"

  qbsSearchPaths: "qbs"

  property string libRoot: path + "/lib"
  property string minimumMacosVersion: "10.9"

  references: [
    "src/src.qbs",
    "lib/lib.qbs",
  ]
}
