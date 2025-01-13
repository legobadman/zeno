import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3


Item {
    id: root

    property var model
    property var parentIndex        //对应上层的tab
    property var childCount
    implicitWidth: mainmain_layout.implicitWidth
    implicitHeight: mainmain_layout.implicitHeight

    //Body
    ColumnLayout {
        id: mainmain_layout
        anchors.fill: parent

        Repeater {
            id: repeater
            model: root.model

            //显示一个group name
            delegate: ColumnLayout {
                Layout.fillWidth: true

                Button {
                    Layout.fillWidth: true
                    //获取group标题：
                    // text: root.model.data(root.model.index(index, 0, root.parentIndex))
                    text: name
                    onClicked: propGroup.shown = !propGroup.shown
                    // visible: root.childCount > 1   //只有一个group可能是默认的情况，不予以显示
                }

                //把整个group 显示出来
                //获取group标题：text: root.model.data(group_item.currentIndex)
                ZPropPanel_Group {
                    id: propGroup
                    // property var currentIndex: root.model.index(index, 0, root.parentIndex)
                    property bool shown: true
                    visible: shown
                    //height: implicitHeight
                    height: shown ? implicitHeight : 0
                    Behavior on height {
                        NumberAnimation {
                            easing.type: Easing.InOutQuad
                        }
                    }
                    //Behavior on height { NumberAnimation { duration: 200 } }

                    clip: true

                    Layout.fillWidth: true
                    model: params
                    parentIndex: model.index(index, 0)
                    childCount: model.rowCount()
                }
            }
        }
    }
}