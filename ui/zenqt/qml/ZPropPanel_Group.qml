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
            text: "This is Component A"
            //anchors.fill: parent
        }
    }

    Component {
        id: componentB
        Rectangle {
            color: "lightgreen"
            Text {
                text: "This is Component B"
                anchors.centerIn: parent
            }
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
        columns: 1  // 每行显示 3 个元素
        rowSpacing: 10
        columnSpacing: 10

        Repeater {
            model: childCount  // 数据模型，创建childCount个子项
            delegate: 
                Loader {
                    //Layout.fillWidth: true
                    sourceComponent: getComponent(index)
                    /*
                    sourceComponent: Text {
                        text: root.model.data(root.model.index(index, 0, parentIndex))
                        color: "black"
                    }
                    */
                }
        }
    }

    // 定义函数，返回不同的组件
    function getComponent(index) {
        return componentA;
    }

/*
    function getParamLabelOrControl(index) {
        return Text { 
            text: root.model.data(root.model.index(index, 0, parentIndex))
            color: "black"
        }
    }

    
    function getParamLabelOrControl(index) {
        if (index % 2 == 0) {
            int row = index / 2
            return paramLabel
        }else{
            return componentB
        }
    }
 
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
    */
}