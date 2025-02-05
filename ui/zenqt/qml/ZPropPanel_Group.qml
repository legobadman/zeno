import QtQuick                   2.15
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Controls.Styles   1.4
import QtQuick.Dialogs 1.3
import zeno.enum 1.0
import Scintilla 1.0
import "./controls"


Item {
    id: root

    property var model          //ParamPlainModel*
    property bool is_input_prim: true

    implicitWidth: 300//mainlayout.implicitWidth      //宽度是由外部决定的，mainlayout的左右锚点要设定为这个，于是这里就给一个初始化宽度。
    implicitHeight: mainlayout.implicitHeight

    Component {
        id: compVec2edit
        ZVec2Editor {
            Layout.fillWidth: true
            anchors.left: parent.left
            anchors.right: parent.right
                        
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
            Layout.fillWidth: true
            anchors.left: parent.left
            anchors.right: parent.right

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
            Layout.fillWidth: true
            anchors.left: parent.left
            anchors.right: parent.right

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
            Layout.fillWidth: true
            anchors.left: parent.left
            anchors.right: parent.right

            text: mvalue

            onEditingFinished: {
                root.model.setData(mindex, text, Model.ROLE_PARAM_QML_VALUE)
            }
        }
    }

    Component {
        id: textedit
        ScrollView {
            id: scrollView
            focus: true
            clip: true

            implicitWidth: 300
            implicitHeight: 300

            Layout.fillWidth: true
            anchors.left: parent.left
            anchors.right: parent.right

            property alias quickScintillaEditor: quickScintillaEditor
            property bool actionFromKeyboard: false

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            // ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            // ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ScintillaEditBase {
                id: quickScintillaEditor

                anchors.fill: parent

                property int minContentWidth: 1200       //如果拖动宽度小于这个值，就设定为这个值，其余部分以滚动方式显示, 否则，拖动宽度越大，控件组宽度就跟随变大
                //implicitWidth: minContentWidth          //TODO: 其实可以为每个内部组件设置最小大小，然后以某种方式计算出最小大小，然后设置到这里。

                property int minContentHeight: 1200
                //implicitHeight: minContentHeight

                //implicitWidth: Math.max(minContentWidth, logicalWidth)
                //implicitHeight: Math.max(minContentHeight, logicalHeight)

                //width: Math.max(minContentWidth, logicalWidth)
                //height: Math.max(minContentHeight, logicalHeight)

                // width: {
                //     console.log("editbase width = " + scrollView.availableWidth)
                //     return scrollView.availableWidth //+ 2*quickScintillaEditor.charHeight
                // }
                // height: {
                //     console.log("editbase height = " + scrollView.availableHeight)
                //     return scrollView.availableHeight //+ 2*quickScintillaEditor.charWidth
                // }

                // position of the QuickScintilla controll will be changed in response of signals from the ScrollView
                x : 0
                y : 0

                Accessible.role: Accessible.EditableText

                font.family: "Courier New"  //*/ "Hack"
                font.pointSize: 18
                focus: true
                text: mvalue

                onTextChanged: {
                    // console.log("onTextChanged")
                }

                onFocusChanged: function(focus_in) {
                    if (!focus_in) {
                        if (text != mvalue) {
                            root.model.setData(mindex, text, Model.ROLE_PARAM_QML_VALUE)
                        }
                    }
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
        id: compFileDialog

        FileDialog {
            id: fileDialog
            title: "选择一个文件"
            selectExisting: true // 只允许选择现有文件

            onRejected: {
                
            }
        }
    }

    Component {
        id: compPathEdit

        Row {
            id: controlPathEdit
            spacing: 10

            TextField {
                id: pathField
                width: 250
                placeholderText: "请输入路径或选择文件"
                placeholderTextColor: "gray"
                selectByMouse: true // 启用鼠标选择文本功能
                text: mvalue
                color: "white"
            }

            Button {
                text: "Select File"
                onClicked: {
                    var dialog = compFileDialog.createObject(controlPathEdit, {
                        title: "select file"
                    })
                    dialog.onAccepted.connect(function() {
                        var filePath = dialog.fileUrl.toString()
                        console.log("fileUrl = " + filePath)
                        //TODO: 这种fileUrl是file:// 需要转为普通的路径，否则不好处理
                        root.model.setData(mindex, filePath, Model.ROLE_PARAM_QML_VALUE)
                    })
                    dialog.open()
                }
            }
        }
    }


    Component {
        id: colorDialogComponent
        ColorDialog { }
    }

    function hexToRgb(hex) {
        // 检查是否是有效的 Hex 颜色值
        let match = /^#([0-9a-fA-F]{6})$/.exec(hex);
        if (!match) return null;

        let r = parseInt(match[1].substring(0, 2), 16) / 255.0;
        let g = parseInt(match[1].substring(2, 4), 16) / 255.0;
        let b = parseInt(match[1].substring(4, 6), 16) / 255.0;

        return [r,g,b];
    }

    Component {
        id: compColorWidget

        Rectangle {
            id: colorBlock
            implicitWidth: 96
            implicitHeight: 24
            Layout.fillWidth: true
            anchors.left: parent.left
            anchors.right: parent.right

            color: Qt.rgba(mvalue[0], mvalue[1], mvalue[2], 1)//"red"
            radius: 2

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    var dialog = colorDialogComponent.createObject(colorBlock, {
                        title: "select color"
                    })
                    dialog.onAccepted.connect(function() {
                        var rgbval = hexToRgb(dialog.color)
                        root.model.setData(mindex, rgbval, Model.ROLE_PARAM_QML_VALUE)
                    })
                    dialog.open()
                }
            }

            Component.onCompleted: {
                console.log("mvalue: " + mvalue)
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
        anchors.left: parent.left                  //我们支持动态拓宽面板时，也增大控件宽度，于是这里要锚定parent
        anchors.right: parent.right

        Repeater {
            id: repeater_id
            model: root.model     //关联的是ParamPlainModel或者ParamsModel
            property string maxName: model.maxLengthName

            delegate:
                RowLayout {
                    spacing: 0
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
                            id: icon_image
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

                        TextMetrics {
                            id: textMetrics
                            text: repeater_id.maxName
                        }

                        Layout.preferredWidth: {
                            return Math.max(textMetrics.width, 50) + 16
                        }
                        Layout.alignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                // console.log("socket name onClicked")
                                parent.forceActiveFocus();
                            }
                        }
                    }

                    Loader {
                        Layout.fillWidth: true
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
                            else if (control == ParamControl.ColorVec) {
                                return compColorWidget
                            }
                            else if (control == ParamControl.ReadPathEdit || control == ParamControl.WritePathEdit) {
                                return compPathEdit
                            }
                            else if (control == ParamControl.Slider){
                                return compSlider
                            }
                            else{
                                return nullControl
                            }
                        }
                        property var mvalue: value
                        property var m_control_properties: control_properties
                        property var mindex: per_index
                    }
                }
        }

        Item {
            Layout.fillHeight: true
            width: 10
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