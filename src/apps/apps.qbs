import qbs

Project {

  name: "Apps"

  references: [
    "launcher/launcher.qbs",
    "domtrader/domtrader.qbs",
    "instrument-selector/instrument-selector.qbs",
    "pcleaner/pcleaner.qbs"
  ]

}
