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
import QtQuick.Controls          2.3
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan
import zeno.enum 1.0
import Zeno 1.0

Qan.GraphView {
    id: graphView
    property variant graphModel
    navigable   : true
    resizeHandlerColor: "#03a9f4"       // SAMPLE: Set resize handler color to blue for 'resizable' nodes
    gridThickColor: Qt.rgba(46/255, 46/255, 46/255, 1.0)// Material.theme === Material.Dark ? "#4e4e4e" : "#c1c1c1"
    //grid: null

    //internal property:
    property var edgesobj

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

    //background
    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(31/255, 31/255, 31/255, 1)
        z: -10
    }

    Component {
        id: edgeComponent
        Edge {
            visible: true
            color: "#5FD2FF"
            thickness: 4

            onClicked_outof_curve: function () {
                graphView.clear_temp_edge()
            }
            onClicked: function () {
                //临时边可能会点击到自身的鼠标区域，如果不被别的socket捕获，就认为是点击grid
                graphView.clear_temp_edge()             
            }
        }
    }

    function clear_temp_edge() {
        if (graphView.tempEdge != undefined && graphView.tempEdge.visible) {
            graphView.tempEdge.destroy()
            graphView.tempEdge = null
            graphView.tryconnect = false
        }        
    }

    function invalide_edgearea_clicked() {
        console.log("invalide_edgearea_clicked")
        clear_temp_edge()
    }

    function temp_edge_close() {
        //console.log("边闭合，信息如下：")

        var from_node = tempEdge.tempedge_from_sock["node_name"]
        var from_sock = tempEdge.tempedge_from_sock["sock_name"]
        var from_group = tempEdge.tempedge_from_sock["group"]
        //console.log("from_node = " + from_node + ", from_sock = " + from_sock + ", from_group = " + from_group)

        var to_node = tempEdge.tempedge_to_sock["node_name"]
        var to_sock = tempEdge.tempedge_to_sock["sock_name"]
        var to_group = tempEdge.tempedge_to_sock["group"]
        //console.log("to_node = " + to_node + ", to_sock = " + to_sock + ", to_group = " + to_group)
        
        if (from_group == ParamGroup.OutputObject) {
            graphView.graph.model.addLink(from_node, from_sock, to_node, to_sock)
        }
        else if (from_group == ParamGroup.InputObject){
            graphView.graph.model.addLink(to_node, to_sock, from_node, from_sock)
        }
        else if (from_group == ParamGroup.OutputPrimitive){
            graphView.graph.model.addLink(from_node, from_sock, to_node, to_sock)
        }
        else if (from_group == ParamGroup.InputPrimitive) {
            graphView.graph.model.addLink(to_node, to_sock, from_node, from_sock)
        }
        clear_temp_edge()
    }

    onClicked: {
        //console.log("Graphview.onClicked")
        clear_temp_edge()
        edgesobj.clear_selection()
    }

    onRightClicked: function(pos) {
        //console.log("rightclick: " + pos.x + "," + pos.y)
        newnode_menu.x = pos.x
        newnode_menu.y = pos.y
        newnode_menu.open()
    }

    onHoverInfoChanged: function() {
        var nodeitem = graphView.hoverInfo["node"]
        var hoverpos = graphView.hoverInfo["pos"]

        if (nodeitem) {
            //console.log("hover node: " + nodeitem.node.label)
        }
        if (nodeitem == null || 
                p1_group == ParamGroup.InputPrimitive ||
                p1_group == ParamGroup.OutputPrimitive) {
            //不处理数值类型的吸附，因为本来就不提倡这种用法
            //console.log("no nodeitem hover")
            graphView.tempEdge.point2x = hoverpos.x
            graphView.tempEdge.point2y = hoverpos.y
            graphView.tempEdge.tempedge_to_sock = null    
            return
        }

        var nearest_sock = null
        var p1_group = graphView.tempEdge.p1_group
        var params_repeater = null
        if (p1_group == ParamGroup.InputObject) {
            nearest_sock = nodeitem.getNearestSocket(ParamGroup.OutputObject, hoverpos, graphView.containerItem)
        }
        else if (p1_group == ParamGroup.OutputObject) {
            nearest_sock = nodeitem.getNearestSocket(ParamGroup.InputObject, hoverpos, graphView.containerItem)
        }

        if (nearest_sock) {
            var sock_pos_in_grid = graphView.containerItem.mapFromGlobal(nearest_sock.mapToGlobal(Qt.point(nearest_sock.width/2, nearest_sock.height/2)))
            graphView.tempEdge.point2x = sock_pos_in_grid.x
            graphView.tempEdge.point2y = sock_pos_in_grid.y

            var end_sock_info = {
                "node_name": nearest_sock.nodename,
                "sock_name": nearest_sock.name,
                "group": (p1_group == ParamGroup.InputObject) ? ParamGroup.OutputObject : ParamGroup.InputObject                
            }
            graphView.tempEdge.tempedge_to_sock = end_sock_info
        }
        else{
            graphView.tempEdge.point2x = hoverpos.x
            graphView.tempEdge.point2y = hoverpos.y
            graphView.tempEdge.tempedge_to_sock = null
        }
    }

    onNodeSocketClicked: function(node, group, name, socket_pos_in_grid) {
        //console.log("Qan.GraphView: node: " + node.label + ", group: " + group + ", socket_name:" + name + ",socket_pos_grid:" + socket_pos_in_grid)

        var pos_ = socket_pos_in_grid
        var is_start_to_link = false;
        if (!graphView.tempEdge) {
            graphView.tempEdge = edgeComponent.createObject(graphView.containerItem, {})
            is_start_to_link = true
        }

        if (is_start_to_link) {
            //初始化临时边
            graphView.tempEdge.point1x = pos_.x
            graphView.tempEdge.point1y = pos_.y
            graphView.tempEdge.p1_group = group
            graphView.tempEdge.visible = true

            var start_sock_info = {
                "node_name": node.label,
                "sock_name": name,
                "group": group
            }
            graphView.tempEdge.tempedge_from_sock = start_sock_info
            graphView.tempEdge.tempedge_to_sock = null
            graphView.tryconnect = true
        } else {
            //闭合
            var end_sock_info = {
                "node_name": node.label,
                "sock_name": name,
                "group": group                
            }
            graphView.tempEdge.tempedge_to_sock = end_sock_info
            console.log("闭合边")

            graphView.temp_edge_close()
        }
    }

    graph: Qan.Graph {
        parent: graphView
        id: graph
        model: graphView.graphModel
        nodeDelegate: Qt.createComponent("qrc:/NormalNode.qml")
        selectionColor: Qt.rgba(250/255, 100/255, 0, 1.0)

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
            if (graphView.tempEdge && graphView.tempEdge.tempedge_to_sock) {
                graphView.temp_edge_close()
            }
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

    MenuEventFilter {
        id: menuKeyFilter

        onTextAppended: function(newtext) {
            searchItem.focus = true
            searchItem.forceActiveFocus()

            searchItem.text = searchItem.text + newtext
            // console.log("onTextAppended: " + searchItem.text)
        }
        onTextRemoved: function() {
            if (searchItem.text != "") {
                searchItem.text = searchItem.text.substring(0, searchItem.text.length - 1)
            }
            searchItem.focus = true
            searchItem.forceActiveFocus()
        }
    }

    Menu {
        id: newnode_menu

        property var catemenuitems: []
        property bool catemode: true

        height: childrenRect.height;

        focus: true  // 确保 Menu 可以接收焦点

        TextInput  {
            id: searchItem
            height: 24
            focus: true // 初始设置为焦点

            // onFocusChanged: {
            //     if (!focus) {
            //         searchItem.forceActiveFocus();
            //     }
            // }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    searchItem.forceActiveFocus(); // 鼠标点击时强制聚焦
                }
            }

            onTextChanged: {
                nodecatesmodel.search(text)
            }

            Keys.onReturnPressed: {
                console.log("Enter pressed on TextInput")
                if (newnode_menu.count > 1) {
                    newnode_menu.itemAt(1).triggered()
                        // console.log("count = " + newnode_menu.count)
                        // console.log("itemAt = " + newnode_menu.itemAt(1))
                        // console.log("itemAt = " + newnode_menu.itemAt(1))
                }
            }

            Keys.onPressed: {
                if (event.key === Qt.Key_Up) {
                    console.log("Enter Up")
                } else if (event.key === Qt.Key_Down) {
                    console.log("Enter Down")
                }
            }
        }

        Component {
            id: comp_catemenu
            Menu {
                id: catemenu
                property var _nodelist: nodelist
                property bool ismenu: true

                title: name

                onAboutToShow: {
                    if (loader1.sourceComponent == null) {
                        loader1.sourceComponent = menuItemComponent1
                    }
                }

                // Loader 组件用于动态加载 MenuItem
                Loader {
                    id: loader1
                }

                // 预定义的组件，动态加载时使用
                Component {
                    id: menuItemComponent1
                    Instantiator {
                        model: catemenu._nodelist
                        delegate: MenuItem {
                            text: modelData
                            onTriggered: {
                                console.log(text + " triggered")
                            }
                            Component.onCompleted: {
                            }
                        }
                        onObjectAdded: catemenu.addItem(object)
                    }
                }

                Component.onCompleted: {
                    // 创建一个 EventFilter 实例
                    menuKeyFilter.listenTo(catemenu)
                }
            }            
        }

        Component {
            id: comp_searchitem
            MenuItem {
                property bool ismenu: false
                text: name

                Keys.onReturnPressed: {
                    console.log("Enter pressed on " + name)
                    triggered()
                }

                onTriggered: {
                    console.log(name + " onTriggered")
                }
            }
        }

        Instantiator {
            model: nodecatesmodel       //global model

            delegate: {
                if (searchItem.text == "") {
                    return comp_catemenu;
                }
                else {
                    return comp_searchitem;
                }
            }

            // The trick is on those two lines
            onObjectAdded: {
                if (object.ismenu) {
                    newnode_menu.addMenu(object)
                }
                else {
                    if (newnode_menu.count == 1) {
                        object.highlighted = true
                    }
                    newnode_menu.addItem(object)
                }
            }
            onObjectRemoved: {
                if (object.ismenu) {
                    newnode_menu.removeMenu(object)
                }
                else {
                    newnode_menu.removeItem(object)
                }
            }
        }

        onAboutToHide: {
            searchItem.text = "";
        }

        Component.onCompleted: {
            menuKeyFilter.listenTo(newnode_menu)
        }
    }

    ToolTip { id: toolTip; timeout: 2500 }
    function notifyUser(message) { toolTip.text=message; toolTip.open() }

    Component {
        id: edgescomp
        EdgesContainer {
            onInvalidarea_clicked: function() {
                graphView.invalide_edgearea_clicked();
            }
        }
    }

    Component.onCompleted: {
        navigator.reset_paths()

        edgesobj = edgescomp.createObject(graphView.containerItem, {
            "graphModel": graphView.graphModel,
            "graphView": graphView
        });
        if (edgesobj === null) {
            console.error("Failed to create edges object")
        }
    }

    Component {
        id: myNavigator
        Text {
            id: navigatoritem
            color: Qt.rgba(125/255, 125/255, 125/255, 1)
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
                    parent.color =  Qt.rgba(125/255, 125/255, 125/255, 1)
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

