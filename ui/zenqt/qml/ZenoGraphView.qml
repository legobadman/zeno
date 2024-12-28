/*
 Copyright (c) 2008-2023, Benoit AUTHEMAN All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author or Destrat.io nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan

Qan.GraphView {
    id: graphView
    property variant graphModel
    navigable   : true
    resizeHandlerColor: "#03a9f4"       // SAMPLE: Set resize handler color to blue for 'resizable' nodes
    gridThickColor: Material.theme === Material.Dark ? "#4e4e4e" : "#c1c1c1"

    signal navigateRequest(var lst)
    signal nodeClicked(var node)

    PinchHandler {
        target: null
        onActiveScaleChanged: {
            console.error('centroid.position=' + centroid.position)
            console.error('activeScale=' + activeScale)
            var p = centroid.position
            var f = activeScale > 1.0 ? 1. : -1.
            graphView.zoomOn(p, graphView.zoom + (f * 0.03))
        }
    }

    property var tempEdge: undefined

    Component {
        id: edgeComponent
        Edge {
            visible: true
            color: "#5FD2FF"
            thickness: 4
        }
    }

    MouseArea {
        //似乎只是为了拿到鼠标坐标，和临时边绑定
        id: graphEditorArea
        anchors.fill: parent
        hoverEnabled: false // 允许捕获鼠标悬停事件，这个很重要，没有这个，那么PositionChanged必须要按下鼠标才触发，但另一方面这个会filter掉节点组件的Hover事件，所以只能给临时边拿坐标用

        onPositionChanged: {
             //var globalPosition = mapToGlobal(mouse.x, mouse.y);
             //console.log("全局鼠标位置:", globalPosition.x, globalPosition.y);
            mouse.accepted = false; // 让事件继续传播到 C++
        }
        onPressed: {
            //这个点击无法区分是析构临时边，还是点击了socket，需要针对事件的对象进行处理，比如点击网格是navigable的事件
            mouse.accepted = false;
        }
        onReleased: {
            mouse.accepted = false;
        }
        onClicked:{
            mouse.accepted = false;
        }
        onEntered:{
        }
        onExited:{
        }
    }

    onClicked: {
        if (graphView.tempEdge != undefined && graphView.tempEdge.visible) {
            graphView.tempEdge.destroy()
            graphEditorArea.hoverEnabled = false    //需要把hover置灰，否则节点的hover事件会被editorarea拦截
        }
    }

    onNodeSocketClicked: function(node, group, name, socket_pos_in_grid) {
        //console.log("Qan.GraphView: node: " + node.label + ", group: " + group + ", socket_name:" + name + ",socket_pos_grid:" + socket_pos_in_grid)

        var pos_ = socket_pos_in_grid
        var is_start_to_link = false;
        if (!graphView.tempEdge) {
            graphView.tempEdge = edgeComponent.createObject(graphView.containerItem, {})
            graphEditorArea.hoverEnabled = true
            is_start_to_link = true
        }

        if (is_start_to_link) {
            //初始化临时边
            graphView.tempEdge.point1x = pos_.x
            graphView.tempEdge.point1y = pos_.y
            graphView.tempEdge.point2x = Qt.binding(function() {
                                    //console.log("Qt.binding")
                                    var mouseInContainer = graphView.containerItem.mapFromItem(graphView, Qt.point(graphEditorArea.mouseX, graphEditorArea.mouseY))
                                    //留一定的空隙，否则在释放的时候会触发临时边的鼠标事件
                                    return mouseInContainer.x - 3
                                })
            graphView.tempEdge.point2y = Qt.binding(function() {
                                    var mouseInContainer = graphView.containerItem.mapFromItem(graphView, Qt.point(graphEditorArea.mouseX, graphEditorArea.mouseY))
                                    return mouseInContainer.y - 3
                                })
            graphView.tempEdge.p1_group = group
            graphView.tempEdge.visible = true
        } else {
            //闭合
            console.log("闭合边")
            graphView.tempEdge.destroy()
            graphEditorArea.hoverEnabled = false    //需要把hover置灰，否则节点的hover事件会被editorarea拦截
        }
    }

    graph: Qan.Graph {
        parent: graphView
        id: graph
        model: graphView.graphModel
        nodeDelegate: Qt.createComponent("qrc:/NormalNode.qml")

        Component.onCompleted: {
            //graph.model.name()

            //var n1 = graph.insertNode()
            //n1.label = "Hello World"; n1.item.x=15; n1.item.y= 25
            //n1.item.ratio = 0.4
            //var n2 = graph.insertNode()
            //n2.label = "Node 2"; n2.item.x=15; n2.item.y= 125

            //var e = graph.insertEdge(n1, n2);
            //defaultEdgeStyle.lineType = Qan.EdgeStyle.Curved
        }
        onNodeClicked: function(node, pos) {
            graphView.nodeClicked(node)
            //console.log("node pos: " + pos)
            //notifyUser( "Node <b>" + node.label + "</b> clicked" )
            //nodeEditor.node = node
        }
        onNodeRightClicked: function(node) { notifyUser( "Node <b>" + node.label + "</b> right clicked" ) }
        onNodeDoubleClicked: function(node) { notifyUser( "Node <b>" + node.label + "</b> double clicked" ) }
        onNodeMoved: function(node) { notifyUser("Node <b>" + node.label + "</b> moved") }

        onNodeSocketClicked: function(node, group, name, socket_pos) {
        }
    } // Qan.Graph

    Menu {      // Context menu demonstration
        id: contextMenu
        property var node: undefined
        MenuItem {
            text: "Insert Node"
            onClicked: {
                let n = graph.insertNode()
                n.label = 'New Node'
                n.item.x = contextMenu.x
                n.item.y = contextMenu.y
            }
        }
        MenuItem {
            text: "Remove node"
            enabled: contextMenu.node !== undefined
            onClicked: {
                graph.removeNode(contextMenu.node)
                contextMenu.node = undefined
            }
        }
        onClosed: { // Clean internal state when context menu us closed
            contextMenu.node = undefined
        }
    } // Menu

    onRightClicked: function(pos) {
        //console.log("rightclick: " + pos.x + "," + pos.y)
        contextMenu.x = pos.x
        contextMenu.y = pos.y
        contextMenu.open()
    }

    ToolTip { id: toolTip; timeout: 2500 }
    function notifyUser(message) { toolTip.text=message; toolTip.open() }

    Component {
        id: edgescomp
        EdgesContainer {

        }
    }

    Component.onCompleted: {
        navigator.reset_paths()

        var edgesobj = edgescomp.createObject(graphView.containerItem, {
            "graphModel": graphView.graphModel,
            "graphView": graphView,
            "z": 100
        });
        if (edgesobj === null) {
            console.error("Failed to create edges object")
        }
    }

    Component {
        id: myNavigator
        Text {
            id: navigatoritem
            color: "#000000"
            font.pixelSize: 20
            font.family: "微软雅黑"
            signal graphitemClicked(int idx)
            property int inneridx
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: {
                    navigatoritem.graphitemClicked(parent.inneridx)
                }
                onEntered: {
                    parent.color = "#0078D4"
                    parent.font.underline = true;
                }
                onExited: {
                    parent.color = "#000000"
                    parent.font.underline = false;
                }
            }
        }
    }

    //子图路径导航栏
    Row {
        id: navigator
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.top: parent.top
        anchors.topMargin: 15
        spacing: 8

        function reset_paths() {
            const lst = graphView.graphModel.path()
            for (let i = navigator.children.length - 1; i >= 0; i--) {
                let child = navigator.children[i];
                if (child !== null) {
                    child.destroy(); // 销毁子元素
                }
            }
            for (var i = 0; i < lst.length; i++)
            {
                var graphitem = myNavigator.createObject(navigator, { text: lst[i], inneridx: i });
                graphitem.graphitemClicked.connect(function onGraphitemClicked(inneridx) {
                    //inneridx是点击的文本是第几个，比如 main > Subnet1，如果点击Subnet1则是1，main就是0
                    var path_list = []
                    for (let i = 0; i < navigator.children.length; i++) {
                        let child = navigator.children[i];
                        if (child.text != ">") {
                            path_list.push(child.text);
                            if (inneridx == 0) {
                                graphView.navigateRequest(path_list)    //发送信号到上一层(ZenoSubnetsView，跳转图层)
                                break;
                            }
                            --inneridx;
                        }
                    }
                })
                if (i < lst.length - 1) {
                    var seperatoritem = myNavigator.createObject(navigator, { text: ">", inneridx: i });
                }
            }
        }
    }

    Loader {
        id: proppaneLoader
        anchors.bottom: parent.bottom; anchors.bottomMargin: 15
        anchors.right: parent.right; anchors.rightMargin: 15
        property var selectedNode: undefined
        visible: false

        sourceComponent: ZPropPanel {
            id: proppanel
            node:  proppaneLoader.selectedNode     
        }
    }

    Connections {
        target: graphView
        onNodeClicked: function(node){
            proppaneLoader.active = false
            proppaneLoader.selectedNode = node
            proppaneLoader.active = true
        }

    }
}  // Qan.GraphView

