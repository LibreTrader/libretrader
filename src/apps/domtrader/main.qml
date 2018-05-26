import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

ApplicationWindow {

  id: root
  visible: true
  width: 300
  height: 600
  color: "#1b1f25"
  title: ""
  property var prevInstrument
  property bool cleaningUp: false

  function finished() {
    console.debug("unsubscribed2!")
    close()
  }

  onClosing: function(close) {
    if (cleaningUp) {
      close.accepted = true
      return
    }
    cleaningUp = true
    subscriber.unsubscribeAndFinish(finished)
    close.accepted = false
  }

  RowLayout {
    id: topRow

    height: 40
    spacing: 2
    anchors.margins: 2
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right

    MyComboBox {
      id: cmbInstrument
      Layout.fillWidth: true
      model: instrumentsModel
      onCurrentIndexChanged: {
        var idx = currentIndex
        console.log(idx)
        var ins
        if (idx >= 0) {
          ins = instrumentsModel.jsObject(idx)
          root.title = "DOM - " + ins["Name"]
        }
        else {
          ins = {"Name": ""}
        }
        prevInstrument = ins
        subscriber.setSubscription(idx)
      }
    }

  }

  ListView {

    id: view
    anchors.top: topRow.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.margins: 2

    focus: true
    clip: true
    model: domModel

    property var rowHeight: 45

    property var center: function() {
      var numRows = Math.round(height / rowHeight)
      if (count < numRows) {
        currentIndex = 0
      } else {
        currentIndex = domModel.centerIndex() - Math.round(numRows / 2)
      }
      positionViewAtIndex(currentIndex, ListView.Beginning)
    }

    Keys.onSpacePressed: center()
    Keys.onPressed: {
      var step
      var idx = currentIndex
      if (event.key === Qt.Key_PageDown) {
        step = Math.round((height / rowHeight) * 0.9)
        idx = Math.min(currentIndex + step, count - 1)
      } else if (event.key === Qt.Key_PageUp) {
        step = Math.round((height / rowHeight) * 0.9)
        idx = Math.max(currentIndex - step, 0)
      } else if (event.key === Qt.Key_Down) {
        idx += 1
      } else if (event.key === Qt.Key_Up) {
        idx -= 1
      }
      else {
        event.accepted = false
        return
      }
      var numRows = Math.round(height / rowHeight)
      currentIndex = Math.max(Math.min(idx, count - numRows), 0)
      positionViewAtIndex(currentIndex, ListView.Beginning)
      event.accepted = true
    }

    Connections {
      target: domModel
      onSnapshotReceived: view.center()
    }

    //        MouseArea {
    //            anchors.fill: parent
    //            onClicked: parent.focus = true
    //        }

    onFlickStarted: {
      focus = true
    }

    onContentYChanged: {
      if (!flicking) {
        return
      }
      var yDiff = contentY - originY
      currentIndex = Math.round(yDiff / rowHeight)
    }

    onCurrentIndexChanged: {
//      if (flicking) {
//        return
//      }
//      console.log("STOP")
//      console.log(currentIndex)
//      var numRows = Math.round(height / rowHeight)
//      currentIndex = Math.max(Math.min(currentIndex, count - numRows), 0)
//      positionViewAtIndex(currentIndex, ListView.Beginning)
    }

    //        add: Transition {
    //            SequentialAnimation {
    //                PauseAnimation {
    //                    duration: 100
    //                }
    //                NumberAnimation {
    //                    property: "opacity"
    //                    from: 0
    //                    to: 1
    //                    duration: 100
    //                }
    //            }
    //        }

    //        remove: Transition {
    //            NumberAnimation {
    //                property: "textScale"
    //                from: 1
    //                to: 0
    //                duration: 100
    //            }
    //        }

    displaced: Transition {
      //            SequentialAnimation {
      //                PauseAnimation {
      //                    duration: 100
      //                }
      NumberAnimation {
        properties: "x,y"
        duration: 100
        easing.type: Easing.OutQuad
        easing.amplitude: 5
      }
      //            }
    }

    delegate: Component {

      Rectangle {
        id: row
        width: view.width
        height: view.rowHeight
        color: "transparent"
        // border.color: "#cec9c0"
        property int spacing: 1
        property color textColor: "white"
        property double textScale: 1

        MouseArea {
          anchors.fill: parent
          onClicked: {
            console.log("item clicked: " + index)
            // view.currentIndex = index
            view.focus = true
          }
        }

        Rectangle {
          id: bidCell
          width: (parent.width - priceCell.width) / 2
          anchors.left: row.left
          anchors.top: row.top
          anchors.bottom: row.bottom
          border.width: 1
          //                        border.color: "#cec9c0"
          //                        color: "#0001bf"
          color: "#123669"
          Text {
            id: priceText
            visible: hasBid
            anchors.centerIn: parent
            text: {
              var s = ""
              if (bidSize > 0) {
                s += bidSize + " "
              }
              if (impliedBidSize > 0) {
                s += "(" + impliedBidSize + ")"
              }
              return s
            }
            font.bold: true
            color: row.textColor
            scale: row.textScale
            font.pixelSize: 12
          }
        }

        Rectangle {
          id: priceCell
          width: (parent.width - parent.spacing) / 2.5
          anchors.left: bidCell.right
          anchors.leftMargin: row.spacing
          anchors.top: row.top
          anchors.bottom: row.bottom
          border.width: 1
          //                        border.color: "#cec9c0"
          color: "#323a45"
          Text {
            anchors.centerIn: parent
            text: price
            color: row.textColor
            scale: row.textScale
            font.bold: true
            font.pixelSize: 12
          }
        }

        Rectangle {
          id: askCell
          width: (parent.width - priceCell.width) / 2
          anchors.left: priceCell.right
          anchors.leftMargin: row.spacing
          anchors.top: row.top
          anchors.bottom: row.bottom
          border.width: 1
          //                        border.color: "#cec9c0"
          //                        color: "#a20000"
          color: "#93112f"
          Text {
            visible: hasAsk
            anchors.centerIn: parent
            text: {
              var s = ""
              if (askSize > 0) {
                s += askSize + " "
              }
              if (impliedAskSize > 0) {
                s += "(" + impliedAskSize + ")"
              }
              return s
            }
            color: row.textColor
            scale: row.textScale
            font.bold: true
            font.pixelSize: 12
          }
        }
      }
    }
  }

  Component.onCompleted: {
    function insModelReset() {
      if (!prevInstrument) {
        return
      }
      var idx = instrumentsModel.indexOf(prevInstrument)
      cmbInstrument.currentIndex = idx
    }
    instrumentsModel.onModelReset.connect(insModelReset)
  }
}
