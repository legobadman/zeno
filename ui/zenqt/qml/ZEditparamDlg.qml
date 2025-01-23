import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import zeno.enum 1.0
import "./controls"
import "./view"
import QtQuick.Window 2.15

// Dialog {
//     id: dialog
//     title: "参数设置"
//     width: 800
//     height: 600
//     modal: true

//     property var node: undefined

//     contentItem: Row {  // 水平布局分为三部分
//         anchors.fill: parent

//         // 左侧列表视图
//         ListView {
//             id: leftListView
//             width: parent.width * 0.2
//             height: parent.height
//             model: ListModel {
//                 ListElement { name: "Tab" }
//                 ListElement { name: "Group" }
//                 ListElement { name: "Integer" }
//                 ListElement { name: "Float" }
//             }
//             delegate: Item {
//                 width: parent.width
//                 height: 30
//                 Text {
//                     anchors.centerIn: parent
//                     text: model.name
//                 }
//             }
//         }

//         TreeView {
//             id: styledTreeView

//             Layout.fillHeight: true

//             model: ListModel {
//                 ListElement { name: "Tab" }
//                 ListElement { name: "Group" }
//                 ListElement { name: "Integer" }
//                 ListElement { name: "Float" }
//             }
//             selectionEnabled: true
//             hoverEnabled: true

//             color: "#AAAACC"
//             handleColor: "#B0CCCC"
//             hoverColor: "#2A2D2E"
//             selectedColor: "#37373D"
//             selectedItemColor: "white"
//             handleStyle: TreeView.Handle.TriangleOutline
//             rowHeight: 40
//             rowPadding: 30
//             rowSpacing: 12
//             font.pixelSize: 20

//             onCurrentIndexChanged: {
//                 //main图现在都是默认打开的。
//                 if (currentIndex == null) {
//                     return;
//                 }
//                 var graphM = model.graph(currentIndex)
//                 if (graphM == null) {
//                     console.log("warning: graphM is null");
//                     return;
//                 }

//                 var path_list = graphM.path()
//                 graphs_tabbar.currentIndex = 0;     //切换到主图的view
//                 stack_main_graphview.jumpTo(path_list);

//                 //var ident = model.ident(currentIndex)
//                 //var owner = graphM.owner()
//                 //console.log("ident: " + ident)
//                 //console.log("owner: " + owner)
//                 //tabView打开标签为owner的图，并且把焦点focus在ident上。
//                 //app.tab.activatePage(owner, graphM)
//             }
//             onCurrentDataChanged: {
//                 //console.log("current data is " + currentData)
//             }
//             onCurrentItemChanged: {
//                 //console.log("current item is " + currentItem)
//             }
//         }

//         // // 中间部分
//         // Column {
//         //     width: parent.width * 0.5
//         //     height: parent.height
//         //     spacing: 10

//             // // 树形视图
//             // TreeView {
//             //     anchors.fill: parent
//             //     model: ListModel {
//             //         ListElement {
//             //             name: "Item 1"
//             //         }
//             //         ListElement {
//             //             name: "Item 2"
//             //         }
//             //     }

//             //     TableViewColumn {
//             //         role: "name"
//             //         title: "Name"
//             //         width: 300
//             //     }
//             // }

//             // // 下方三个列表视图
//             // ListView {
//             //     id: listView1
//             //     height: 80
//             //     model: ListModel { ListElement { name: "output1" } }
//             //     delegate: Text { text: model.name }
//             // }

//             // ListView {
//             //     id: listView2
//             //     height: 80
//             //     model: ListModel { ListElement { name: "objInput1" } }
//             //     delegate: Text { text: model.name }
//             // }

//             // ListView {
//             //     id: listView3
//             //     height: 80
//             //     model: ListModel { ListElement { name: "objOutput1" } }
//             //     delegate: Text { text: model.name }
//             // }
//         // }

//         // 右侧部分
//         Column {
//             width: parent.width * 0.3
//             spacing: 10

//             TextField {
//                 id: inputName
//                 placeholderText: "名称"
//             }

//             TextField {
//                 id: inputLabel
//                 placeholderText: "标签"
//             }

//             ComboBox {
//                 id: socketProperty
//                 model: ["Property 1", "Property 2", "Property 3"]
//                 currentIndex: 0
//             }
//         }
//     }

//     // 标准对话框按钮
//     footer: Row {
//         spacing: 10
//         Button {
//             text: "应用"
//             onClicked: console.log("应用点击")
//         }
//         Button {
//             text: "取消"
//             onClicked: dialog.close()
//         }
//     }
// }






// 自定义对话框
Window {
    id: dialog
    width: 800
    height: 600
    visible: false
    title: "对话框"
    flags: Qt.Dialog | Qt.WindowTitleHint | Qt.WindowCloseButtonHint // 设置窗口类型为对话框，带标题栏和关闭按钮
    modality: Qt.ApplicationModal  // 将窗口设置为模态
    property var node: undefined

    // 对话框内容
    Rectangle {
        width: parent.width
        height: parent.height
        // color: "lightgray"

        // Text {
        //     text: "这是一个带标题栏的可移动对话框。\n您确定要继续吗？"
        //     anchors.centerIn: parent
        //     horizontalAlignment: Text.AlignHCenter
        //     wrapMode: Text.WordWrap
        // }

        RowLayout {  // 水平布局分为三部分
            anchors.fill: parent
        
            // 左侧列表视图
            ListView {
                id: leftListView
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3

                model: ListModel {
                    ListElement { name: "Tab" }
                    ListElement { name: "Group" }
                    ListElement { name: "Integer" }
                    ListElement { name: "Float" }
                }
                delegate: Item {
                    width: parent.width
                    height: 30
                    Text {
                        anchors.centerIn: parent
                        text: model.name
                    }
                }
            }

            // 中间部分
            Item {//使用item做一层隔离，否则会报：QML QQuickLayoutAttached: Binding loop detected for property循环绑定警告
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3

                ColumnLayout {
                    // 下方三个列表视图
                    anchors.fill: parent

                    Loader {
                        id: treeviewLoader
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: parent.height * 2 / 5
                        sourceComponent: {
                            return treeview
                        }
                    }
                    ListView {
                        id: listView1
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: parent.height / 5
                        model: ListModel { ListElement { name: "output1" } }
                        delegate: Text { text: model.name }
                    }

                    ListView {
                        id: listView2
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: parent.height / 5
                        model: ListModel { ListElement { name: "objInput1" } }
                        delegate: Text { text: model.name }
                    }

                    ListView {
                        id: listView3
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.preferredHeight: parent.height / 5
                        model: ListModel { ListElement { name: "objOutput1" } }
                        delegate: Text { text: model.name }
                    }
                }
            }

            // 右侧部分
            ColumnLayout {
                spacing: 10
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width / 3

                TextField {
                    id: inputName
                    placeholderText: "名称"
                }

                TextField {
                    id: inputLabel
                    placeholderText: "标签"
                }

                ComboBox {
                    id: socketProperty
                    model: ["Property 1", "Property 2", "Property 3"]
                    currentIndex: 0
                }
            }

        }

        RowLayout {
            spacing: 20
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20

            Button {
                text: "确定"
                onClicked: {
                    console.log("点击了确定")
                    dialog.visible = false
                }
            }

            // Button {
            //     text: "取消"
            //     onClicked: {
            //         console.log("点击了取消")
            //         dialog.visible = false
            //     }
            // }
        }
    }

    function openDialog() {
        dialog.visible = true
    }

    Component {
        id: treeview
        // ZPropPanel {
        //     id: proppanel
        //     node: dialog.node
        // }
        Item {
            id: treeviewItem
            anchors.fill: parent
            property var customuimodel : dialog.node.params.customUIModel().tabModel()

            property int currentIndex: -1 // 当前右键点击的条目索引
            Menu {
                id: contextMenu
                property var curidx : undefined
                MenuItem {
                    text: "新增条目"
                    onTriggered: {
                        curidx.insert(currentIndex + 1, { name: "New Item", selected: false });
                    }
                }
                MenuItem {
                    text: "删除条目"
                    onTriggered: {
                        curidx.remove(currentIndex);
                    }
                }
            }

            ScrollView {
                anchors.fill: parent
                ScrollBar.vertical.policy: ScrollBar.AlwaysOn

                clip: true
                ColumnLayout {
                    id: treeLayout
                    anchors.fill: parent
                    // width: parent.width // 确保宽度与 ScrollView 一致
                    Repeater {
                        model: customuimodel
                        delegate: Loader {
                            // Layout.fillWidth: true
                            Layout.preferredWidth: parent.width // 确保填满父容器宽度

                            sourceComponent: tabComp
                            onLoaded: {
                                item.tabname = tabname
                                item.groupmodel = groups
                                // item.indent = 0
                            }
                        }
                    }
                }
                
            }

            Component {
                id: triangleArrow
                Canvas {
                    id: triangleCanvas
                    width: 20
                    height: 20
                    signal clicked() // 自定义信号
                    property bool isExpanded: true // 初始状态为展开
                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height); // 清除画布
                        
                        // 设置旋转中心为画布的中心
                        ctx.save();
                        ctx.translate(width / 2, height / 2); // 移动到中心点
                        ctx.rotate(rotationAngle);  // 旋转三角形
                        
                        // 绘制三角形（初始指向右侧）
                        ctx.beginPath();
                        ctx.moveTo(2, 0);  // 右顶点
                        ctx.lineTo(-4, -6); // 左上角
                        ctx.lineTo(-4, 6);  // 左下角
                        ctx.closePath();
                        
                        ctx.fillStyle = "gray"; // 设置填充颜色
                        ctx.fill(); // 填充三角形
                        ctx.restore();  // 恢复状态
                    }

                    property real rotationAngle: Math.PI / 2 // 初始角度为90

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton
                        onClicked: {
                            // 切换展开/收起状态
                            triangleCanvas.isExpanded = !triangleCanvas.isExpanded;
                            // 根据展开状态来旋转
                            if (triangleCanvas.isExpanded) {
                                triangleCanvas.rotationAngle += Math.PI / 2; // 收起时逆时针旋转90度
                            } else {
                                triangleCanvas.rotationAngle -= Math.PI / 2; // 展开时顺时针旋转90度
                            }
                            // groupComp.visible = triangleCanvas.isExpanded; // 控制子项可见性
                            triangleCanvas.requestPaint(); // 请求重绘
                            triangleCanvas.clicked();
                        }
                    }
                }
            }
            // treeNode：用于显示每个节点及其子节点
            Component {
                id: tabComp

                ColumnLayout {
                    Layout.fillWidth: true
                    property string tabname: ""
                    property var groupmodel: undefined

                    Rectangle {
                        //width: parent.width
                        height: 20
                        color: "lightgray"
                        Layout.fillWidth: true
                        Row {
                            spacing: 10
                            anchors.fill: parent
                            
                            Loader {
                                id: tabtriangleArrowLoader
                                height: parent.height
                                // Layout.fillWidth: true
                                anchors.verticalCenter: parent.verticalCenter
                                sourceComponent: triangleArrow
                            }
                            Connections {
                                target: tabtriangleArrowLoader.item  // 这里使用 checkboxLoader.item 来引用加载后的复选框组件
                                function onClicked() {
                                    groupComp.visible = tabtriangleArrowLoader.item.isExpanded
                                }
                            }
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                text: tabname
                            }
                        }
                    }

                    // 子节点递归显示
                    ColumnLayout {
                        id: groupComp
                        Layout.fillWidth: true
                        Layout.leftMargin: 20 // 设置缩进
                        visible: true

                        Repeater {
                            model: groupmodel
                            delegate:  

                            ColumnLayout {
                                Layout.fillWidth: true
                                // Layout.leftMargin: 20 // 设置缩进

                                Rectangle {
                                    //width: parent.width
                                    height: 20
                                    color: "lightblue"
                                    Layout.fillWidth: true
                                    // Layout.leftMargin: 20 // 设置缩进

                                    Row {
                                        spacing: 10
                                        anchors.fill: parent

                                        Loader {
                                            id: grouptriangleArrowLoader
                                            height: parent.height
                                            // Layout.fillWidth: true
                                            anchors.verticalCenter: parent.verticalCenter
                                            sourceComponent: triangleArrow
                                        }
                                        Connections {
                                            target: grouptriangleArrowLoader.item  // 这里使用 checkboxLoader.item 来引用加载后的复选框组件
                                            function onClicked() {
                                                paramComp.visible = grouptriangleArrowLoader.item.isExpanded
                                            }
                                        }
                                        Text {
                                            anchors.verticalCenter: parent.verticalCenter
                                            text: groupname
                                        }
                                    }
                                }
                                ColumnLayout {
                                    id: paramComp
                                    Layout.fillWidth: true
                                    Layout.leftMargin: 20 // 设置缩进
                                    property string nodeName: ""
                                    property int indent: 20

                                    Repeater {
                                        model: params
                                        delegate: Rectangle {
                                            id:param
                                            //width: parent.width
                                            height: 20
                                            color: "lightgreen"
                                            Layout.fillWidth: true
                                            // Layout.leftMargin: 40 // 设置缩进

                                            Row {
                                                spacing: 10
                                                anchors.fill: parent
                                                Text {
                                                    //id: toggle
                                                    anchors.verticalCenter : parent.verticalCenter
                                                    text : paramname
                                                }
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                acceptedButtons: Qt.AllButtons
                                                onClicked: {
                                                    // // 单击选中条目
                                                    // for (let i = 0; i < childrenNodes.count; i++) {
                                                    //     childrenNodes.setProperty(i, "selected", false);
                                                    // }
                                                    // childrenNodes.setProperty(index, "selected", true);
                                                }

                                                // 右键弹出菜单
                                                onPressed: if (mouse.button === Qt.RightButton) {
                                                    currentIndex = index; // 保存当前条目索引
                                                    contextMenu.x = mouse.x + param.mapToItem(treeviewItem, 0, 0).x; // 使用全局坐标
                                                    contextMenu.y = mouse.y + param.mapToItem(treeviewItem, 0, 0).y;
                                                    contextMenu.open();
                                                }
                                            }


                                        }
                                    }
                                }

                            }
                        }
                    }
                }
            }
        }
    }
}