import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import zeno.enum 1.0
import "./controls"


Pane {
    id: comp
    property var node: undefined
    property var nodeItem: undefined
    property bool use_tabview: false
    //implicitWidth: panel_loader.implicitWidth
    //implicitHeight: panel_loader.implicitHeight
    implicitWidth: 300
    implicitHeight: 300
    padding: 0

    onNodeChanged: nodeItem = node ? node.item : undefined

    background: Rectangle {
        // color: "transparent" // 设置 Pane 的背景颜色
        color: Qt.rgba(51./255, 51./255, 51./255, 1.0)
    }

    Component {
        id: compBlank
        Item {
            visible: false
        }
    }

    // 定义参数的各个组件，包括参数名，socket和控件
    Component {
        id: paramText
        Row {
            ToolButton {
                id: btn_show_prim_sock
                checkable: true
                checked: socket_visible
                property bool reentry: false
                icon.source: checked ? "qrc:/icons/parameter_key-frame_correct.svg" : "qrc:/icons/parameter_key-frame_idle.svg"

                onClicked: {

                }

                onCheckedChanged: {
                    //model.setData(mindex, btn_show_prim_sock.checked == true, Model.ROLE_PARAM_SOCKET_VISIBLE)
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
                text: name    //默认是DisplayRole，参数名
                color: Qt.rgba(210.0/255, 210.0/255, 210.0/255, 1.0) //"white"
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
            value: mvalue
        }
    }

    Component {
        id: compVec3edit
        ZVec3Editor {
            value: mvalue
        }
    }

    Component {
        id: compVec4edit
        ZVec4Editor {
            value: mvalue
        }
    }

    Component {
        id: complineedit
        ZLineEditor {
            text: mvalue
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
        }
    }

    Component {
        id: compCheckbox
        CheckBox {
            id: checkbox
            checkState: mvalue ? Qt.Checked : Qt.Unchecked
        }
    }

    Component {
        id: nullControl
        Text {
            text: ""
        }
    }

    Component {
        id: compPropertyPane
        Frame {
            id: base_tabview
            anchors.fill: parent
            property var treemodel: comp.node ? comp.node.params.customParamModel() : undefined
            property var tabsindex: treemodel ? treemodel.index(0, 0) : undefined
            property var outputsindex: treemodel ? treemodel.index(1, 0) : undefined
            property var customuimodel: comp.node ? comp.node.params.customUIModel() : undefined

            background: Rectangle {
                color: "transparent" // 背景颜色（可以设置为透明或其他颜色）
                border.color: "white" // 设置边框颜色
                border.width: 0 // 设置边框宽度
            }

            Component {
                id: comp_plain_proppane
                ScrollView {
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                    ScrollBar.vertical.policy: ScrollBar.AsNeeded

                    ColumnLayout {
                        Repeater {
                            model: comp.node.params
                            delegate:
                                RowLayout {
                                    Text {
                                        text: name
                                        color: "white"
                                        visible: group == ParamGroup.InputPrimitive
                                    }
                                    Loader {
                                        sourceComponent: {
                                            if (group != ParamGroup.InputPrimitive) {
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
                                                return nullControl
                                            }
                                            else{
                                                return nullControl
                                            }
                                        }
                                        property var mvalue: value
                                        property var m_control_properties: control_properties
                                    }
                                }
                        }
                    }
                }
            }

            Component {
                id: comp_tabbase_proppane
                ColumnLayout {
                    id: tablayout
                    //anchors.fill: parent

                    Text {
                        text: comp.node.label;
                        color: Qt.rgba(210.0/255, 210.0/255, 210.0/255, 1.0)//"white"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "white"
                    }

                    TabBar {
                        id: property_tabbar
                        property int tabcount: base_tabview.treemodel.rowCount(base_tabview.tabsindex)
                        visible: tabcount > 1
                        Repeater {
                            model: property_tabbar.tabcount
                            delegate: TabButton {
                                property var mtabindex: base_tabview.treemodel.index(index, 0, base_tabview.tabsindex)
                                text: base_tabview.treemodel.data(mtabindex)
                                width: 100
                            }
                        }
                    }

                    StackLayout {
                        id: panel_stack
                        currentIndex: property_tabbar.currentIndex

                        Repeater {
                            model: base_tabview.treemodel.rowCount(base_tabview.tabsindex)
                            delegate: ScrollView {
                                clip: true
                                ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                                ZPropPanel_Tab {
                                    model: base_tabview.treemodel
                                    parentIndex: model.index(index, 0, base_tabview.tabsindex)
                                    childCount: model.rowCount(parentIndex)
                                }
                            }
                        }
                    }

                    //输出的prim类型的参数组，主要是为了让用户勾选显示/隐藏输出socket.
                    ZPropPanel_Group {
                        id: propGroup
                        height: implicitHeight
                        Behavior on height {
                            NumberAnimation {
                                easing.type: Easing.InOutQuad
                            }
                        }
                        //Behavior on height { NumberAnimation { duration: 200 } }

                        clip: true

                        Layout.fillWidth: true
                        model: base_tabview.treemodel
                        parentIndex: outputsindex
                        childCount: base_tabview.treemodel.rowCount(outputsindex)
                        visible: childCount > 0
                    }
                }
            }

            Loader {
                anchors.fill: parent
                sourceComponent: {
                    if (customuimodel != null) {
                        return comp_tabbase_proppane
                    }
                    else{
                        return comp_plain_proppane
                    }
                }
            }
        }
    }

    Loader {
        id: panel_loader
        anchors.fill: parent

        sourceComponent: {
            return comp.nodeItem ? compPropertyPane : compBlank
        }
    }
}