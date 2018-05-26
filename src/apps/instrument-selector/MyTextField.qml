import QtQuick 2.10
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3

TextField {
  id: root
  width: 250
  height: 40
  hoverEnabled: true
  verticalAlignment: Text.AlignVCenter
  font.pixelSize: 14
  color: "#707070"
  property alias value: root.text
  property string fieldName
  property alias toolTip: myToolTip.text

  function setValue(val) { text = val; }

  ToolTip {
    id: myToolTip
    delay: 1000
    timeout: 5000
    visible: text ? root.hovered : false
    font.pixelSize: 12
  }

  background: Rectangle {
    border.color: root.enabled ? "#303842" : "transparent"
    color: root.enabled ? "transparent" : "#303842"
    radius: 2
  }
}

