import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Controls.Styles   1.4
import zeno.enum 1.0
import "./controls"


Item {
    id: root

    property var model          //ParamPlainModel*
    property bool is_input_prim: true

    implicitWidth: mainlayout.implicitWidth
    implicitHeight: mainlayout.implicitHeight

    Component {
        id: compVec2edit
        ZVec2Editor {
            value: mvalue

            onEditingFinished: {
                var vec = get_value()
                root.model.setData(mindex, vec, Model.ROLE_PARAM_QML_VALUE)                
            }
        }
    }

    Component {
        id: compVec3edit
        ZVec3Editor {
            value: mvalue

            onEditingFinished: {
                var vec = get_value()
                root.model.setData(mindex, vec, Model.ROLE_PARAM_QML_VALUE)
            }
        }
    }

    Component {
        id: compVec4edit
        ZVec4Editor {
            value: mvalue
            onEditingFinished: {
                var vec = get_value()
                root.model.setData(mindex, vec, Model.ROLE_PARAM_QML_VALUE)
            }
        }
    }

    Component {
        id: complineedit
        ZLineEditor {
            text: mvalue

            onEditingFinished: {
                root.model.setData(mindex, text, Model.ROLE_PARAM_QML_VALUE)
            }
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
                text: mvalue

                background: Rectangle {
                    color: "white" // 背景颜色
                    border.color: textArea.activeFocus ? "blue" : "gray"  // 焦点时改变边框颜色
                    border.width: 1
                }

                onEditingFinished: {
                    root.model.setData(mindex, text, Model.ROLE_PARAM_QML_VALUE)
                }
            }
        }
    }

    Component {
        id: compCombobox
        ComboBox {
            id: comboboxitem
            model: m_control_properties["combobox_items"]
            currentIndex: {
                return model.indexOf(mvalue)
            }
            onCurrentTextChanged: {
                //这个条件是为了过滤初始化值时的value changed
                if (currentText != mvalue) {
                    root.model.setData(mindex, currentText, Model.ROLE_PARAM_QML_VALUE)
                }
            }
        }
    }

    Component {
        id: compSlider
        RowLayout {
            ZLineEditor {
                text: "0"
                width: 40
                property alias val: slideritem.value
                onEditingFinished: {
                    val = text
                }
                onValChanged: {
                    text = val
                }
            }            
            Slider {
                id: slideritem
                property var list: m_control_properties["slider"]
                from: list[0]
                value: mvalue
                to: list[1]
                stepSize: list[2]

                onValueChanged: {
                    //这个条件是为了过滤初始化值时的value changed
                    if (value != mvalue) {
                        // console.log("slider value changed")
                        root.model.setData(mindex, value, Model.ROLE_PARAM_QML_VALUE)
                    }
                }
            }
        }
    }

    Component {
        id: compCheckbox
        CheckBox {
            id: checkbox
            checkState: mvalue ? Qt.Checked : Qt.Unchecked

            onCheckStateChanged: {
                root.model.setData(mindex, checkState == Qt.Checked, Model.ROLE_PARAM_QML_VALUE)
            }
        }
    }

    Component {
        id: nullControl
        Text {
            text: ""
        }
    }

    //Body
    ColumnLayout {
        id: mainlayout
        spacing: 0
        Layout.fillWidth: true

        Repeater {
            model: root.model     //关联的是ParamPlainModel
            delegate:
                RowLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignLeft
                    Layout.fillWidth: true

                    property bool is_match_group: (root.is_input_prim && group == ParamGroup.InputPrimitive) || (!root.is_input_prim && group == ParamGroup.OutputPrimitive)

                    ToolButton {
                        id: btn_show_prim_sock
                        checkable: true
                        checked: socket_visible
                        icon.source: checked ? "qrc:/icons/parameter_key-frame_correct.svg" : "qrc:/icons/parameter_key-frame_idle.svg"
                        visible: is_match_group
                        Layout.leftMargin: 0

                        onCheckedChanged: {
                            root.model.setData(root.model.index(index, 0), btn_show_prim_sock.checked == true, Model.ROLE_PARAM_SOCKET_VISIBLE)
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
                        text: name  /* c++导出的名字, 可到 ParamPlainModel::roleNames()查看 */
                        color: "white"
                        visible: is_match_group
                        Layout.preferredWidth: 128      //TODO：calculate maximum width by all params.
                        Layout.alignment: Qt.AlignLeft

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                // console.log("socket name onClicked")
                                parent.forceActiveFocus();
                            }
                        }
                    }
                    Loader {
                        sourceComponent: {
                            if (!is_match_group) {
                                return compBlank;
                            }
                            if (control == ParamControl.Lineedit){
                                return complineedit
                            }
                            else if (control == ParamControl.Combobox){
                                return compCombobox
                            }
                            else if (control == ParamControl.Multiline){
                                return textedit
                            }
                            else if (control == ParamControl.Checkbox){
                                return compCheckbox
                            }
                            else if (control == ParamControl.Vec2edit){
                                return compVec2edit
                            }
                            else if (control == ParamControl.Vec3edit){
                                return compVec3edit
                            }
                            else if (control == ParamControl.Vec4edit){
                                return compVec4edit
                            }
                            else if (control == ParamControl.CodeEditor){
                                return textedit
                            }
                            else if (control == ParamControl.Slider){
                                return compSlider
                            }
                            else{
                                return nullControl
                            }
                        }
                        property var mvalue: value /* value是c++导出的名字 */
                        property var m_control_properties: control_properties /* value是c++导出的名字 */
                        property var mindex: per_index
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                // console.log("empty area of RowLayout onClicked")
                                parent.forceActiveFocus();
                            }
                        }
                    }
                }
        }
    }

    // 背景点击区域
    Item
    {
        z: -100
        anchors.fill: parent
        MouseArea {
            anchors.fill: parent
            onClicked: {
                //console.log("empty area of Group Item")
                parent.forceActiveFocus();
            }
        }        
    }
}