import QtQuick                   2.12
import QtQuick.Controls          2.15
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Shapes            1.0
import QtQuick.Controls.Styles   1.4
import Qt.labs.settings 1.1

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan
import Zeno 1.0 as Zen
import "./view"


Item {
    id: rootgraphsview

    Component {
       id: myTabButton
       CustomTabButton {
           
       }
    }

    ToolBar {
        id: zeditortoolbar
        width: parent.width
        property bool view_reentry: false       //用于记录treeview和listview之间互相通知的重入标志

        background: Rectangle { // 自定义背景颜色
            color: "#1F1F1F"  // 设置背景颜色
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.topMargin: 0
            anchors.bottomMargin: 0
            spacing: 0

            ToolButton {
                id: assets_list
                checkable: true
                checked: false
                
                icon.source: hovered || checked  ? "qrc:/icons/subnet-listview-on.svg" : "qrc:/icons/subnet-listview.svg"

                onClicked: {

                }

                onCheckedChanged: {
                    if (!zeditortoolbar.view_reentry) {
                        zeditortoolbar.view_reentry = true

                        tree_list.checked = false
                        stack_main_or_asset.visible = checked
                        stack_main_or_asset.currentIndex = 0

                        zeditortoolbar.view_reentry = false
                    }
                }    

                contentItem: Image {
                    id: icon_image
                    source: parent.icon.source
                    sourceSize.width: 20
                    sourceSize.height: 20
                    smooth: true
                    antialiasing: true
                    //anchors.verticalCenter: parent.verticalCenter
                }
                
                background: Rectangle {
                    x: icon_image.x
                    y: icon_image.y
                    width: 20
                    height: 20
                    opacity: enabled ? 1 : 0.3
                    color: parent.hovered || parent.checked ? "#4F5963" : "transparent"
                    border.color: parent.down ? "#17a81a" : "#21be2b"
                    border.width: 0
                    radius: 2
                }
                
            }

            ToolButton {
                id: tree_list
                icon.source: hovered || checked  ? "qrc:/icons/nodeEditor_nodeTree_selected.svg" : "qrc:/icons/nodeEditor_nodeTree_unselected.svg"
                checkable: true
                checked: true
                property bool reentry: false

                onClicked: {

                }

                onCheckedChanged: {
                    if (!zeditortoolbar.view_reentry) {
                        zeditortoolbar.view_reentry = true

                        assets_list.checked = false
                        stack_main_or_asset.visible = checked
                        stack_main_or_asset.currentIndex = 1
                        
                        zeditortoolbar.view_reentry = false
                    }
                }    

                contentItem: Image {
                    source: parent.icon.source
                    sourceSize.width: 20
                    sourceSize.height: 20
                    smooth: true
                    antialiasing: true
                    //anchors.verticalCenter: parent.verticalCenter
                }

                background: Rectangle {
                    x: icon_image.x
                    y: icon_image.y
                    width: 20
                    height: 20
                    opacity: enabled ? 1 : 0.3
                    color: parent.hovered || parent.checked ? "#4F5963" : "transparent"
                    border.color: parent.down ? "#17a81a" : "#21be2b"
                    border.width: 0
                    radius: 2
                }
            }

            Item { Layout.fillWidth: true }
        }
    }    

    Rectangle {
        id: speratorline
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: zeditortoolbar.bottom
        height: 1
        color: "black"
    }
    
    SplitView {
        id: mainLayout
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: speratorline.bottom
        anchors.bottom: parent.bottom

        spacing: 10
        orientation: Qt.Horizontal

        handle: Item {
            implicitWidth: 2

            Rectangle {
                implicitWidth: 2
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height
                color: SplitHandle.hovered ? "#00ff00" : "#2B2B2B"
            }
        }

        StackLayout {
            id: stack_main_or_asset
            clip: true
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: 1

            Rectangle {
                id: assets_block
                width: 200
                implicitWidth: 200
                Layout.maximumWidth: 400
                color: "#181818"
                Layout.fillHeight: true

                // 1.定义delegate，内嵌三个Text对象来展示Model定义的ListElement的三个role
                Component {
                    id: assetItemDelegate
                    Item {
                        id: wrapper
                        width: parent.width
                        height: 30

                        // 实现了鼠标点选高亮的效果
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: {
                                wrapper.ListView.view.currentIndex = index
                                var idx = assetsModel.index(index, 0)
                                var assetname = assetsModel.data(idx)
                                var asset_graph_model = assetsModel.getAssetGraph(assetname)

                                for (var i = 0; i < graphs_tabbar.count; i++) {
                                    let tab = graphs_tabbar.itemAt(i);
                                    if (tab.text == assetname) {
                                        //已存在tab，直接激活即可
                                        graphs_tabbar.currentIndex = i;
                                        return;
                                    }
                                }

                                graphs_tabbar.addItem(myTabButton.createObject(graphs_tabbar, {text: assetname, width: 200}))
                                const graphsview_comp = Qt.createComponent("qrc:/ZenoGraphView.qml")
                                const newgraphview = graphsview_comp.createObject(graphs_stack, {graphModel: asset_graph_model})
                                graphs_tabbar.currentIndex = graphs_tabbar.count - 1;
                                //TODO: delete and adjust index
                            }
                        }

                        RowLayout {
                            anchors.left: parent.left
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.leftMargin: 10
                            spacing: 8

                            Text {
                                id: lbl_assetname
                                text: classname
                                // 是否是当前条目
                                color: wrapper.ListView.isCurrentItem ? "white" : "#C3D2DF"
                                font.pixelSize: 22
                                Layout.preferredWidth: 120
                            }
                        }
                    }
                }
                    
                // 2.定义ListView
                ListView {
                    id: listView
                    anchors.fill: parent

                    // 使用先前设置的delegate
                    delegate: assetItemDelegate
                        
                    // 3.ListModel专门定义列表数据的，它内部维护一个 ListElement 的列表。
                    model: assetsModel

                    // 背景高亮
                    focus: true
                    highlight: Rectangle{
                        color: "#3D3D3D"
                    }
                }
            }

            Rectangle {
                SplitView.preferredWidth: 340
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: "#181818"

                TreeView {
                    id: styledTreeView

                    Layout.fillHeight: true

                    model: treeModel
                    selectionEnabled: true
                    hoverEnabled: true

                    color: "#AAAACC"
                    handleColor: "#B0CCCC"
                    hoverColor: "#2A2D2E"
                    selectedColor: "#37373D"
                    selectedItemColor: "white"
                    handleStyle: TreeView.Handle.TriangleOutline
                    rowHeight: 40
                    rowPadding: 30
                    rowSpacing: 12
                    font.pixelSize: 20

                    onCurrentIndexChanged: {
                        //main图现在都是默认打开的。
                        graphs_tabbar.currentIndex = 0;

                        //var graphM = model.graph(currentIndex)
                        //var ident = model.ident(currentIndex)
                        //var owner = graphM.owner()
                        //console.log("ident: " + ident)
                        //console.log("owner: " + owner)
                        //tabView打开标签为owner的图，并且把焦点focus在ident上。
                        //app.tab.activatePage(owner, graphM)
                    }
                    onCurrentDataChanged: {
                        //console.log("current data is " + currentData)
                    }
                    onCurrentItemChanged: {
                        //console.log("current item is " + currentItem)
                    }
                }
            }
        }

        Item {
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            Layout.fillWidth: true;
            Layout.fillHeight: true

            TabBar {
                id: graphs_tabbar
                TabButton {
                    text: qsTr("main")
                    width: implicitWidth
                }
                
                /*
                CustomTabButton {
                    text: qsTr("Asset1")
                    width: implicitWidth
                }
                CustomTabButton {
                    text: qsTr("Asset2")
                    width: implicitWidth
                }
                */
            }

            StackLayout {
                id: graphs_stack
                width: parent.width
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: graphs_tabbar.bottom
                anchors.bottom: parent.bottom
                currentIndex: graphs_tabbar.currentIndex

                ZenoGraphView {
                    graphModel: nodesModel
                }

                /*
                Item {
                    Loader { anchors.fill: parent; source: "qrc:/zenographview.qml";  }
                }
                Rectangle {
                    id: homeTab
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "red"
                }
                Rectangle {
                    id: activityTab
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "blue"
                }
                */
            }
        }
    }
}