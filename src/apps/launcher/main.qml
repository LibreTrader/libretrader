import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3
import QtQuick.Controls 2.3

ApplicationWindow {
    id: root
    visible: true
    minimumWidth: 300
    maximumWidth: minimumWidth
    minimumHeight: 40
    maximumHeight: minimumHeight
    title: "LibreTrader"
    property bool closeConfirmed: false

    onClosing: function(close) {

        var numProcesses = appLauncher.processCount()

        if (!closeConfirmed && numProcesses > 0)
        {
            close.accepted = false
            var s = numProcesses + " processes running. "
            s += "Are you sure you want to exit?"
            closeDialog.text = s
            closeDialog.open()
        }
        else {
            close.accepted = true
        }
    }

    MessageDialog {
        id: closeDialog
        title: "Confirm close"
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            root.closeConfirmed = true
            root.close()
        }
    }

    ToolBar {
        anchors.fill: parent
        RowLayout {
            spacing: 5
            ToolButton {
                leftPadding: 40
                Image {
                    source: "images/icon_ins-sel.png"
                    width: 30
                    height: 30
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                }
                onClicked: {
                    appLauncher.launchInstrumentSelector()
                }
            }
            ToolButton {
                Image {
                    id: img_dom
                    source: "images/icon_dom.png"
                    width: 30
                    height: 30
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                }
                onClicked: {
                    appLauncher.launchDomTrader()
                }
            }
        }
    }

    Component.onCompleted: {
    }
}
