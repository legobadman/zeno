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
                    anchors.fill: parent

                    ZPropPanel_Group {
                        id: propGroup
                        visible: true
                        height: implicitHeight
                        clip: true
                        Layout.fillWidth: true
                        model: comp.node.params     //关联的是ParamsModel
                    }

                    Item {
                        //通过设置一个Item来接收空白处的点击事件，从而迫使编辑框等控件的焦点可以丢失，完成编辑
                        z: -30
                        anchors.fill: parent
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                //console.log("empty area of ScrollView")
                                parent.forceActiveFocus();
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
                        property int tabcount: base_tabview.customuimodel.tabModel().rowCount()
                        // visible: tabcount > 1
                        Repeater {
                            model: base_tabview.customuimodel.tabModel()
                            delegate: TabButton {
                                property var mtabindex: base_tabview.customuimodel.tabModel().index(index, 0)
                                text: name
                                width: 100
                            }
                        }
                    }

                    StackLayout {
                        id: panel_stack
                        currentIndex: property_tabbar.currentIndex

                        Repeater {
                            model: base_tabview.customuimodel.tabModel()
                            delegate: ScrollView {
                                clip: true
                                ScrollBar.horizontal.policy: ScrollBar.AsNeeded
                                ScrollBar.vertical.policy: ScrollBar.AsNeeded

                                ZPropPanel_Tab {
                                    model: groups   //ParamTabModel::roleNames()
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
                        model: base_tabview.customuimodel.outputModel()
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