import QtQuick 2.8
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

Rectangle {

    id: root
    width: 250
    height: 40
    color: "transparent"

    property alias model: control.model
//    property var onCurrentIndexChanged: function() {}
    property alias currentText: control.currentText
    property alias currentIndex: control.currentIndex
    property alias value: control.currentText
    property string fieldName: ""

    function setValue(val) {
      for (var i = 0; i < connectors.length; i++) {
        if (val ===  control.textAt(i)) {
          control.currentIndex = i
          return
        }
      }
      console.warn(val + " not found in ComboBox")
      control.currentIndex = -1
    }

    ComboBox {
        id: control
        anchors.fill: parent

        indicator: Canvas {
            id: myCanvas
            x: control.width - width - control.rightPadding
            y: control.topPadding + (control.availableHeight - height) / 2
            width: 12
            height: 8
            contextType: "2d"

            Connections {
                target: control
                onPressedChanged: myCanvas.requestPaint()
            }

            onPaint: {
                context.reset();
                context.moveTo(0, 0);
                context.lineTo(width, 0);
                context.lineTo(width / 2, height);
                context.closePath();
                context.fillStyle = control.pressed ? "#31383B" : "#303842";
                context.fill();
            }
        }

        contentItem: Text {
            leftPadding: 10
            rightPadding: control.indicator.width + control.spacing
            text: control.displayText
            font: control.font
            color: "#303842" //"#31383B" // : "#303842"
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            implicitWidth: 10
            implicitHeight: 40
            border.color: control.pressed ? "#001C26" : "#303842"
            border.width: control.visualFocus ? 2 : 1
            radius: 2
        }

        popup: Popup {
            y: control.height - 1
            width: control.width
            implicitHeight: contentItem.implicitHeight
            padding: 1

            contentItem: ListView {
                clip: true
                implicitHeight: contentHeight
                model: control.popup.visible ? control.delegateModel : null
                currentIndex: control.highlightedIndex

                ScrollIndicator.vertical: ScrollIndicator { }
            }

            background: Rectangle {
                border.color: "#263D3D"
                radius: 3
            }
        }

    }
}

