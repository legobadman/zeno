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
            model: root.childCount

            //显示一个group name
            delegate: 
                    //把整个group 显示出来
                    //获取group标题：text: root.model.data(group_item.currentIndex)
                    ZPropPanel_Group {
                        property var currentIndex: root.model.index(index, 0, root.parentIndex)
                        Layout.fillWidth: true
                        model: root.model
                        parentIndex: currentIndex
                        childCount: root.model.rowCount(currentIndex)
                    }
        }
    }
}