import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3


Item {
    id: root

    property var model
    property var parentIndex
    property var childCount

    implicitWidth: mainmain_layout.implicitWidth
    implicitHeight: mainmain_layout.implicitHeight

    //Body
    ColumnLayout {
        id: mainmain_layout
        anchors.fill: parent
        
        Repeater {
            id: repeater
            model: childCount

            delegate:
                Text {
                    text: root.model.data(root.model.index(index, 0, parentIndex))
                }
        }
    }
}