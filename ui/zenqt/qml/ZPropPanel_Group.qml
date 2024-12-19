import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Controls.Styles   1.4
import zeno.enum 1.0
import "./controls"


Item {
    id: root

    property var model
    property var parentIndex
    property var childCount

    implicitWidth: mainmain_layout.implicitWidth
    implicitHeight: mainmain_layout.implicitHeight

    // 定义不同组件
    Component {
        id: paramText
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
        id: compVec2edit
        ZVec2Editor {
            
        }
    }

    Component {
        id: compVec3edit
        ZVec3Editor {

        }
    }

    Component {
        id: compVec4edit
        ZVec4Editor {
            
        }
    }

    Component {
        id: complineedit
        ZLineEditor {

        }
    }

    Component {
        id: compCheckbox
        ZCheckBox {

        }
    }

    Component {
        id: nullControl
        Text {
            text: ""
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
        rowSpacing: 5
        columnSpacing: 30

        Repeater {
            model: childCount * 2  // 数据模型，创建childCount个子项
            delegate: 
                Loader {
                    sourceComponent: {
                        var realindex = index / 2
                        var mindex = root.model.index(realindex, 0, parentIndex)
                        var qvar = root.model.data(mindex, 270)
                        if (index % 2 == 0) {
                            return paramText
                        } 
                        else if (qvar == ParamControl.Lineedit){
                            return complineedit
                        }
                        else if (qvar == ParamControl.Combobox){
                            return nullControl
                        }
                        else if (qvar == ParamControl.Multiline){
                            return nullControl
                        }
                        else if (qvar == ParamControl.Checkbox){
                            return compCheckbox
                        }
                        else if (qvar == ParamControl.Vec2edit){
                            return compVec2edit
                        }
                        else if (qvar == ParamControl.Vec3edit){
                            return compVec3edit
                        }
                        else if (qvar == ParamControl.Vec4edit){
                            return compVec4edit
                        }
                        else if (qvar == ParamControl.CodeEditor){
                            return nullControl
                        }
                        else if (qvar == ParamControl.Slider){
                            return nullControl
                        }
                        else{
                            console.log("qvar = " + qvar)
                            console.log("control = " + ParamControl.Vec3edit)
                            return componentB
                        }
                    }
                    //Loader binds properties to the source component
                    //passed to the delegate
                    property int _index: index / 2
                }
        }
    }
}