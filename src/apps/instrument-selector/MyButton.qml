import QtQuick 2.6
import QtQuick.Controls 2.1

Button {
    id: control
    text: qsTr("Button")
    property color txtColorDown: "#D9D9D9"
    property color txtColorUp: "#000000"
    property color bgColorDown: "#31383B"
    property color bgColorUp: "#EEEEEE"
    property color borderColor: "#000000"
    property int borderWidth: 0
    property int btnRadius: 3
    property int btnWidth: 100
    property int btnHeight: 40

    contentItem: Text {
        id: txt
        text: control.text
        font.bold: true
        font.pixelSize: 14
        opacity: enabled ? 1.0 : 0.3

        color: control.down ? txtColorDown : txtColorUp
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: btnWidth
        implicitHeight: btnHeight
        opacity: enabled ? 1 : 0.3
        radius: btnRadius
        border.width: borderWidth
        color: control.down ? bgColorDown : bgColorUp
    }
}

