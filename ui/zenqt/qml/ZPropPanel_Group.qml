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

    implicitWidth: gridlayout.implicitWidth
    implicitHeight: gridlayout.implicitHeight

    // 定义不同组件
    Component {
        id: paramText
        Row {
            ToolButton {
                id: btn_show_prim_sock
                checkable: true
                checked: model.data(mindex, Model.ROLE_PARAM_SOCKET_VISIBLE)
                property bool reentry: false
                icon.source: checked ? "qrc:/icons/parameter_key-frame_correct.svg" : "qrc:/icons/parameter_key-frame_idle.svg"

                onClicked: {

                }

                onCheckedChanged: {
                    model.setData(mindex, btn_show_prim_sock.checked == true, Model.ROLE_PARAM_SOCKET_VISIBLE)
                }    

                contentItem: Image {
                    source: parent.icon.source
                    sourceSize.width: 20
                    sourceSize.height: 20
                    smooth: true
                    antialiasing: true
                    anchors.verticalCenter: parent.verticalCenter
                }

                background: Rectangle {
                    x: icon_image.x + 3
                    y: icon_image.y
                    width: 14
                    height: 14
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: enabled ? 1 : 0.3
                    color: parent.hovered ? "#f0f0f0" : "transparent"
                    radius: width / 2
                }
            }
            Text {
                text: model.data(mindex)    //默认是DisplayRole，参数名
                anchors.verticalCenter: parent.verticalCenter
            }
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
        id: textedit
        ScrollView {
            id: view
            width: 200
            height: 100

            TextArea {
                id: textArea
                anchors.margins: 10
                placeholderText: "请输入文本..."
                font.pixelSize: 16
                wrapMode: TextArea.WordWrap // 自动换行模式
                readOnly: false // 设置为 true 则为只读

                background: Rectangle {
                    color: "white" // 背景颜色
                    border.color: textArea.activeFocus ? "blue" : "gray"  // 焦点时改变边框颜色
                    border.width: 1
                }
            }
        }
    }

    Component {
        id: compCombobox
        ComboBox {
            model: root.model.data(mindex, Model.ROLE_PARAM_CONTROL_PROPS)["combobox_items"]
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

    //Body
    GridLayout {
        id: gridlayout
        anchors.fill: parent
        property int nCol: 2
        columns: nCol  // 每行显示 2 个元素
        rowSpacing: 5
        columnSpacing: 20

        Repeater {
            model: childCount * gridlayout.nCol  // 数据模型，创建childCount个子项
            delegate: 
                Loader {
                    sourceComponent: {
                        var realindex = index / gridlayout.nCol   //index是grid的项数计数，由于只有两列，所以index/2就是模型的项行号
                        var ctrl = root.model.data(root.model.index(realindex, 0, parentIndex), Model.ROLE_PARAM_CONTROL)
                        if (index % gridlayout.nCol == 0) {
                            return paramText
                        } 
                        else if (ctrl == ParamControl.Lineedit){
                            return complineedit
                        }
                        else if (ctrl == ParamControl.Combobox){
                            return compCombobox
                        }
                        else if (ctrl == ParamControl.Multiline){
                            return textedit
                        }
                        else if (ctrl == ParamControl.Checkbox){
                            return compCheckbox
                        }
                        else if (ctrl == ParamControl.Vec2edit){
                            return compVec2edit
                        }
                        else if (ctrl == ParamControl.Vec3edit){
                            return compVec3edit
                        }
                        else if (ctrl == ParamControl.Vec4edit){
                            return compVec4edit
                        }
                        else if (ctrl == ParamControl.CodeEditor){
                            return textedit
                        }
                        else if (ctrl == ParamControl.Slider){
                            return nullControl
                        }
                        else{
                            return nullControl
                        }
                    }
                    //Loader binds properties to the source component
                    //passed to the delegate
                    property var mindex: root.model.index(index / gridlayout.nCol, 0, parentIndex)
                }
        }
    }
}