import QtQuick 2.10
import QtQuick.Window 2.3
import QtQuick.Controls 1.4

import "helpers.js" as Helpers

Window {

  id: root
  title: "Choose Instrument"
  width: 600
  height: 400
  modality: Qt.ApplicationModal

  property var secListGrp
  property var selectedInstrument: null

  Rectangle {
    anchors.fill: parent

    TableView {
      id: tableView
      anchors.fill: parent

      TableViewColumn {
        id: colInstrumentID
        title: "InstrumentID"
        role: "ZMInstrumentID"
        resizable: true
      }

      TableViewColumn {
        id: colSymbol
        title: "Symbol"
        role: "Symbol"
        resizable: true
      }

      TableViewColumn {
        id: colDescription
        title: "Description"
        role: "SecurityDesc"
        resizable: true
      }

      onDoubleClicked: {
        selectedInstrument = secListGrp[row]
        close()
      }

      Keys.onReturnPressed: {
        selectedInstrument = secListGrp[currentRow]
        close()
      }

    }
  }

  Component.onCompleted: {
    //var model = Helpers.createObjSync("MyListModel", root)
    var i
    var model = Qt.createQmlObject("import QtQuick 2.10; import QtQml.Models 2.2; ListModel {}", root)
    var row
    var d
    var hasSymbol = false
    var hasDescription = false
    for (i = 0; i < secListGrp.length; i++) {
      d = secListGrp[i]
      if (!hasSymbol && "Symbol" in d) {
        hasSymbol = true
      }
      if (!hasDescription && "SecurityDesc" in d) {
        hasDescription = true
      }
      model.append(d)
    }
    console.log(JSON.stringify(secListGrp[0]))
    colSymbol.visible = hasSymbol
    colDescription.visible = hasDescription
    tableView.model = model
  }
}
