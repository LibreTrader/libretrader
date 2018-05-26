import QtQuick 2.10
import QtQuick.Window 2.3
import QtQuick.Dialogs 1.3
import QtQuick.Controls 1.4

Window {

  id: root
  width: 600
  height: 400
  title: "Browse Instruments"
  modality: Qt.ApplicationModal

  property string connector
  property string instrumentID

  TreeView {

    model: treeModel
    anchors.fill: parent
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    focus: true

    function trySelectAndQuit(index) {
      var insID = treeModel.getInstrumentID(index)
      if (insID) {
        instrumentID = insID
        close()
      }
    }

    TableViewColumn {
      id: col1
      title: "Name"
      role: "name"
      resizable: true
      width: Math.round(root.width * 0.3)
    }

    TableViewColumn {
      id: col2
      title: "Description"
      role: "description"
      resizable: true
      width: root.width - col1.width
    }

    onDoubleClicked: {
      trySelectAndQuit(index)
    }

    Keys.onReturnPressed: {
      trySelectAndQuit(currentIndex)
    }

    onExpanded: {

      if (treeModel.isPopulated(index)) {
        return
      }

      function dirResponse(msg) {
        // console.debug(JSON.stringify(msg))
        treeModel.populateNode(msg["Body"]["ZMDirEntries"], index)
      }

      var msg = {}
      msg["Header"] = {}
      msg["Header"]["MsgType"] = "ZMd"  // ZMListDirectory
      msg["Header"]["ZMEndpoint"] = connector
      msg["Body"] = {}
      msg["Body"]["ZMPath"] = treeModel.getPath(index)

      dealer.send(msg, dirResponse)
    }

  }

  Component.onCompleted: {
    function dirResponse(msg) {
      treeModel.populateNode(msg["Body"]["ZMDirEntries"])
    }

    var msg = {}
    msg["Header"] = {}
    msg["Header"]["MsgType"] = "ZMd"  // ZMListDirectory
    msg["Header"]["ZMEndpoint"] = connector
    msg["Body"] = {}
    dealer.send(msg, dirResponse)
  }

}
