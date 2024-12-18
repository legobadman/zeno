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

    //定义各个Component
    // 定义不同组件
    Component {
        id: componentA
        Text {
            text: model.data(model.index(_index, 0, parentIndex))
        }
    }

    Component {
        id: componentB
        Text {
            text: "BBB"
        }
    }

    Component {
        id: componentC
        Rectangle {
            color: "lightcoral"
            Text {
                text: "This is Component C"
                anchors.centerIn: parent
            }
        }
    }

    //Body
    GridLayout {
        id: mainmain_layout
        anchors.fill: parent
        columns: 2  // 每行显示 2 个元素
        rowSpacing: 10
        columnSpacing: 10

        Repeater {
            model: childCount * 2  // 数据模型，创建childCount个子项
            delegate: 
                Loader {
                    //Layout.fillWidth: true
                    //sourceComponent: getComponent(index)
                    sourceComponent: {
                        var realindex = index / 2
                        if (index % 2 == 0) {
                            return componentA
                        } else {
                            return componentB
                        }
                    }
                    //Loader binds properties to the source component
                    //passed to the delegate
                    property int _index: index / 2
                }
        }
    }

    // 定义函数，返回不同的组件
    function getComponent(index) {
        return componentA;
    }
}