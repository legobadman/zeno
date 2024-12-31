import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan
import zeno.enum 1.0

Column {
    id: comp
    property bool isEditing: false
    property string text: ""

    Text {
        id: displayText
        visible: !comp.isEditing
        text: comp.text

        MouseArea{
            anchors.fill: parent
            onDoubleClicked: comp.isEditing = true
        }
    }

    TextEdit {
        id: editableText
        visible: comp.isEditing
        text: comp.text

        MouseArea {
            anchors.fill: parent
            onClicked: editableText.forceActiveFocus()
        }
    }
}