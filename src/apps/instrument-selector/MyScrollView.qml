import QtQuick 2.10

Flickable {
    id: root
    Rectangle {
        anchors.right: parent.right
        y: {
            var res = root.visibleArea.yPosition * root.contentHeight
            res += root.visibleArea.yPosition * root.height
            return res
        }
        width: 10
        height: 20
        //height: root.visibleArea.heightRatio * root.height
        color: "black"
    }
    onFlickingChanged: {
        console.log(visibleArea.yPosition)
    }
}
