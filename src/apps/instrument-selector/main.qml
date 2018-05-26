import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtQuick.Dialogs 1.2

import "helpers.js" as Helpers

ApplicationWindow {

  id: root
  visible: true
  width: 720
  minimumWidth: width
  maximumWidth: width
  height: 480
  minimumHeight: height
  maximumHeight: height
  title: "Instrument Selector"

  property var connectors: []
  property var connectorData: Object()
  property var instrumentInfo: null
  property bool firstUpdate: true

  //  function populateInstrumentInfo() {
  //    instrumentInfo["ZMInstrumentID"] = insIDTextField.value
  //  }

  function populateUIFromInstrumentInfo() {
    console.debug("populateUIFromInstrumentInfo")
    var i
    if (instrumentInfo == null) {
      nameTextField.value = ""
      insIDTextField.value = ""
      connectorCmb.currentIndex = -1
      return
    }
    if ("Name" in instrumentInfo) {
      nameTextField.value = instrumentInfo["Name"]
    }
    if ("ZMInstrumentID" in instrumentInfo) {
      insIDTextField.value = instrumentInfo["ZMInstrumentID"]
      // hack to force redraw
      insIDTextField.visible = false
      insIDTextField.visible = true
    }

    if ("ZMEndpoint" in instrumentInfo) {
      // will populate dynamic grid
      connectorCmb.setValue(instrumentInfo["ZMEndpoint"])
      //      for (i = 0; i < connectors.length; i++) {
      //        if (connectors[i] === instrumentInfo["ZMEndpoint"]) {
      //          connectorCmb.currentIndex = i
      //        }
      //      }
    }

    console.debug(settingsColumn.children.length + " items in settingsColumn")
    if (settingsColumn.children.length < 2) {
      return
    }
    var dynGrid = settingsColumn.children[1]
    console.debug(dynGrid.children.length + " items in dynGrid")

    for (var key in instrumentInfo) {
      for (i = 0; i< dynGrid.children.length; i++) {
        var item = dynGrid.children[i]
        console.debug(key + " " + item)
        if (item.fieldName === key) {
          item.setValue(instrumentInfo[key])
        }
      }
    }
    console.log("populatedUI")
  }

  function getSecurityList(callback) {

    console.log("getting security list ...")

    var i

    function secListReceived(msg) {
      console.log(JSON.stringify(msg))
      var dialog
      var args
      var grp
      var showError = false
      if (msg["Header"]["MsgType"] !== "y") {  // SecurityList
        showError = true
      } else {
        grp = msg["Body"]["SecListGrp"]
        console.log("got " + grp.length + " instruments ...")
        if (grp.length === 0) {
          showError = true
        }
      }
      if (showError) {
        console.error("failed getting security list ...")
        args = {
          "title": "Instruments not found",
          "text": "Please try different search criteria.",
          "icon": StandardIcon.Warning,
        }
        dialog = Helpers.createObjSync("MyMessageDialog", root, args)
        return
      }
      if (grp.length > 1) {
        args = {
          "secListGrp": grp,
          "visible": true
        }
        dialog = Helpers.createObjSync("ChooseInstrumentDialog",
                                       root,
                                       args)
        function instrumentChosen(ev) {
          console.log("instrument chosen")
          instrumentInfo = dialog.selectedInstrument
          instrumentInfo["Name"] = nameTextField.value
          instrumentInfo["ZMEndpoint"] = connectorCmb.value
          populateUIFromInstrumentInfo()
          if (callback != null) {
            callback()
          }
        }
        dialog.closing.connect(instrumentChosen)
        return
      }

      // only 1 match
      instrumentInfo = grp[0]
      // MDFunnel should remove ZMTickerID ...
      delete instrumentInfo["ZMTickerID"]
      instrumentInfo["Name"] = nameTextField.value
      instrumentInfo["ZMEndpoint"] = connectorCmb.value
      populateUIFromInstrumentInfo()
      if (callback != null) {
        callback()
      }
    }

    var msg = {}
    msg["Header"] = {}
    msg["Header"]["MsgType"] = "x"  // SecurityListRequest
    msg["Header"]["ZMEndpoint"] = connectorCmb.value
    var body = {}
    if (insIDTextField.value) {
      body["ZMInstrumentID"] = insIDTextField.value
    }
    else {
      var dynGrid = settingsColumn.children[1]
      for (i = 0; i< dynGrid.children.length; i++) {
        var item = dynGrid.children[i]
        if (item.fieldName && item.value) {
          body[item.fieldName] = item.value;
        }
      }
      //      for (i = 1; i < settingsColumn.children.length; i++) {
      //        var item = settingsColumn.children[i]
      //        body[item.fieldName] = item.value
      //      }
    }
    console.debug(JSON.stringify(body))
    msg["Body"] = body
    dealer.send(msg, secListReceived)
  }

  function populateDynamicGrid(connector) {

    console.debug("populating dynamic grid for " + connector + " ...")

    // check that connectorData is ready, schedule retries until ready

    // console.log("populateDynamicGrid: " + JSON.stringify(connectorData))

    var i
    var args
    var conData
    var d

    if (connector) {
      conData = connectorData[connector]
      if (conData == null) {
        Helpers.delayed(10, function() {
          populateDynamicGrid(connector)
        })
        return
      }

      if (!("fields" in conData) || !("canListDir" in conData)) {
        Helpers.delayed(10, function() {
          populateDynamicGrid(connector)
        })
        return
      }
    }

    console.log("starting up...")

    for (i = 1; i < settingsColumn.children.length; i++) {
      settingsColumn.children[i].destroy()
    }

    if (!connector) {
      browseButton.enabled = false
      return
    }

    args = {
      "visible": false,
      "width": settingsColumn.effWidth,
      "columns": 2,
      "rowSpacing": 8,
      "columnSpacing": 50,
    }

    var grid = Helpers.createObjSync("MyGridLayout", settingsColumn, args)

    browseButton.enabled = conData["canListDir"]

    var fields = conData["fields"]

    //console.debug(JSON.stringify(fields))

    for (i = 0; i < fields.length; i++) {
      d = fields[i]
      //console.debug(JSON.stringify(d))
      if (d["type"] === "selection") {
        args = {
          "text": d["label"],
          "Layout.preferredHeight": "staticGrid.rowHeight",
        }
        Helpers.createObjSync("MyLabel", grid, args)
        args = {
          "fieldName": d["field"],
          "model": d["values"],
          "Layout.preferredHeight": staticGrid.rowHeight,
          "Layout.alignment": Qt.AlignRight,
          "Layout.preferredWidth": staticGrid.valueCellWidth,
        }
        Helpers.createObjSync("MyComboBox", grid, args)
      }
      if (d["type"] === "str") {
        args = {
          "text": d["label"],
          "Layout.preferredHeight": "staticGrid.rowHeight",
        }
        Helpers.createObjSync("MyLabel", grid, args)
        args = {
          "fieldName": d["field"],
          "Layout.preferredHeight": staticGrid.rowHeight,
          "Layout.alignment": Qt.AlignRight,
          "Layout.preferredWidth": staticGrid.valueCellWidth,
        }
        Helpers.createObjSync("MyTextField", grid, args)
      }
    }

    //    for (i = 0; i < 20; i++)
    //    {
    //      args = {
    //        "text": "Field " + i + ":",
    //        "Layout.preferredHeight": staticGrid.rowHeight,
    //      }
    //      Helpers.createObjSync("MyLabel", grid, args)
    //      args = {
    //        "Layout.preferredHeight": staticGrid.rowHeight,
    //        "Layout.alignment": Qt.AlignRight,
    //        "Layout.preferredWidth": staticGrid.valueCellWidth,
    //      }
    //      Helpers.createObjSync("MyTextField", grid, args)
    //    }

    grid.visible = true

    if (firstUpdate) {
      populateUIFromInstrumentInfo()
    }
    firstUpdate = false

  }

  Rectangle {
    anchors.fill: parent

    Rectangle {
      id: listRect
      width: root.width / 3
      anchors.left: parent.left
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      color: "#1b1f25"

      ListView {
        id: listInstruments
        model: instrumentsModel
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height - 48
        highlight: Component {
          Rectangle {
            width: listInstruments.width
            height: listInstruments.rowHeight
            color: "black"
            opacity: 0.5
          }
        }
        delegate: listInstrumentsDelegate
        focus: true
        highlightMoveDuration: 1
        onCurrentIndexChanged: {
          if (currentIndex < 0) {
            //instrumentInfo = null
            return
          } else {
            instrumentInfo = instrumentsModel.jsObject(currentIndex)
          }
          populateUIFromInstrumentInfo()
        }
        property int rowHeight: 50
      }

      Component {
        id: listInstrumentsDelegate
        Rectangle {
          width: listInstruments.width
          height: listInstruments.rowHeight
          color: "transparent"
          Text {
            text: modelData
            color: "white"
            font.pixelSize: 14
            font.bold: true
            anchors.fill: parent
            anchors.margins: 10
          }
          MouseArea {
            anchors.fill: parent
            onClicked: listInstruments.currentIndex = index
          }
        }
      }

      RowLayout {
        id: buttonRect
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: listInstruments.bottom
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        spacing: 5

        MyButton {
          Layout.fillWidth: true
          Layout.fillHeight: true
          text: "+"
          bgColorUp: "#123669"
          txtColorUp: "#F0F4F1"
          btnRadius: 5
          onClicked: {
            listInstruments.currentIndex = -1
            instrumentInfo = null
            populateUIFromInstrumentInfo()
          }
        }

        MyButton {
          Layout.fillWidth: true
          Layout.fillHeight: true
          text: "-"
          bgColorUp: "#94112f"
          txtColorUp: "#F0F4F1"
          btnRadius: 5
          onClicked: {
            var idx = listInstruments.currentIndex
            instrumentsModel.remove(idx)
            if (idx < 0 || idx >= listInstruments.count) {
              instrumentInfo = null
            } else {
              instrumentInfo = instrumentsModel.jsObject(
                    listInstruments.currentIndex)
            }
            populateUIFromInstrumentInfo()
          }
        }
      }
    }

    Rectangle {

      id: settingsRect
      border.color: "#303842"
      border.width: 2
      anchors.left: listRect.right
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: footerRect.top

      ScrollView {

        id: scrollView
        anchors.fill: parent
        anchors.margins: parent.border.width

        Flickable {

          anchors.fill: parent
          boundsBehavior: Flickable.StopAtBounds
          clip: true

          Column {

            id: settingsColumn

            anchors.fill: parent
            leftPadding: 50
            rightPadding: 50
            topPadding: 20
            bottomPadding: 20
            spacing: 8

            property int effWidth: scrollView.width - leftPadding
                                   - rightPadding

            GridLayout {

              id: staticGrid
              width: parent.effWidth
              columns: 2
              rowSpacing: 8
              columnSpacing: 50

              property int rowHeight: 40
              property int valueCellWidth: 200

              MyLabel {
                Layout.preferredHeight: parent.rowHeight
                text: "Name:"
              }

              MyTextField {
                id: nameTextField
                Layout.preferredHeight: parent.rowHeight
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: parent.valueCellWidth
                fieldName: "Name"
                toolTip: "Display name of the instrument"
              }

              MyLabel {
                Layout.preferredHeight: parent.rowHeight
                text: "Connector:"
              }

              MyComboBox {
                id: connectorCmb
                Layout.preferredHeight: parent.rowHeight
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: parent.valueCellWidth
                model: connectors
                fieldName: "ZMEndpoint"
                onCurrentTextChanged: {
                  populateDynamicGrid(currentText)
                }
              }

              MyLabel {
                Layout.preferredHeight: parent.rowHeight
                text: "Instrument ID:"
              }

              MyTextField {
                id: insIDTextField
                Layout.preferredHeight: parent.rowHeight
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: parent.valueCellWidth
                fieldName: "ZMInstrumentID"
                toolTip: "Unique instrument ID"
              }

            }
          }


        }
      }
    }

    Rectangle {

      id: footerRect
      anchors.left: listRect.right
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      color: "#303842"
      height: 50

      RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 50
        anchors.rightMargin: 50

        MyButton {
          id: browseButton
          Layout.preferredWidth: 100
          Layout.preferredHeight: 40
          text: "Browse"
          btnRadius: 3
          property var dialog: Object()
          onClicked: {
            var args = {
              "visible": true,
              "connector": connectorCmb.currentText,
            }
            dialog = Helpers.createObjSync("BrowseDialog", root, args)

            function browseDialogClosed(ev) {
              if (!dialog.instrumentID) {
                return
              }
              insIDTextField.value = dialog.instrumentID
              insIDTextField.forceActiveFocus()  // otherwise bug with redraw?
              getSecurityList()
            }
            dialog.closing.connect(browseDialogClosed)
          }
        }

        MyButton {
          id: searchButton
          Layout.preferredWidth: 100
          Layout.preferredHeight: 40
          text: "Search"
          btnRadius: 3
          onClicked: {
            console.debug(JSON.stringify(root.connectorData))
            getSecurityList()
          }
        }

        MyButton {
          id: saveButton
          text: "Save"
          Layout.preferredWidth: 100
          Layout.preferredHeight: 40

          bgColorUp: "#207245"
          txtColorUp: "#F0F4F1"
          btnRadius: 3

          enabled: connectorCmb.value

          onClicked: {
            var i
            var args
            var name = nameTextField.value
            if (!name) {
              args = {
                "title": "Name not specified",
                "text": "Please specify the instrument name.",
                "icon": StandardIcon.Warning,
              }
              Helpers.createObjSync("MyMessageDialog", root, args)
              return
            }

            var oldIns
            if (listInstruments.currentIndex >= 0) {
              oldIns = instrumentsModel.jsObject(
                    listInstruments.currentIndex)
            }
            if (!oldIns || name !== oldIns["Name"]) {
              for (i = 0; i < listInstruments.count; i++) {
                if (instrumentsModel.jsObject(i)["Name"] === name) {
                  args = {
                    "title": "Name already exists",
                    "text": "Please specify different instrument name.",
                    "icon": StandardIcon.Warning,
                  }
                  Helpers.createObjSync("MyMessageDialog", root, args)
                  return
                }
              }
            }

            function secListFinished() {
              console.log("sec list finished")
              var updIdx = instrumentsModel.update(instrumentInfo)
              listInstruments.currentIndex = updIdx
            }
            getSecurityList(secListFinished)
          }
        }
      }

    }

    Component.onCompleted: {

      function endpointsReceived(msg) {

        root.connectors = msg["Body"]["ZMEndpoints"]
        console.debug("Connectors: " + root.connectors)

        for (var i = 0; i < root.connectors.length; i++) {

          var connector = root.connectors[i]
          root.connectorData[connector] = {}


          function insFieldsReceivedWrapper(connector) {
            return function(msg) {
              console.debug("instrument fields on " + connector + ":\n"
                            + JSON.stringify(msg))
              root.connectorData[connector]["fields"] = msg["Body"]
            }
          }

          msg = {}
          msg["Header"] = {}
          msg["Header"]["MsgType"] = "ZMf"  // ZMGetInstrumentFields
          msg["Header"]["ZMEndpoint"] = connector
          msg["Body"] = {}
          dealer.send(msg, insFieldsReceivedWrapper(connector))

          function capsReceivedWrapper(connector) {
            return function(msg) {
              var d = root.connectorData[connector]
              var caps = msg["Body"]["ZMCaps"]
              console.debug("capabilities for " + connector + ":\n"
                            + JSON.stringify(caps))
              d["capabilities"] = caps
              d["canListDir"] = caps.indexOf("LIST_DIRECTORY") >= 0
            }
          }

          msg = {}
          msg["Header"] = {}
          msg["Header"]["MsgType"] = "ZMc"  // ZMListCapabilities
          msg["Header"]["ZMEndpoint"] = connector
          msg["Body"] = {}
          dealer.send(msg, capsReceivedWrapper(connector))
        }
      }

      var msg = {}
      msg["Header"] = {"MsgType": "ZMe"}  // ZMListEndpoints
      msg["Body"] = {}
      dealer.send(msg, endpointsReceived)
    }

  }
}
