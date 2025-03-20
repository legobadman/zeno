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
import zeno.enum 1.0

Qan.NodeItem {
    id: nodeItem
    //width: 110; height: 60
    //x: 15;      y: 15

    // 自动适应子布局的尺寸
    implicitWidth: mainmain_layout.implicitWidth
    implicitHeight: mainmain_layout.implicitHeight;
    resizable: false

    property alias isview : right_status_group.isview
    property alias isbypass: right_status_group.isbypass
    property var nodestatus: 0

    property string defattr: "abc"

    //clip: true        //设置会导致选框不出现

    readonly property real backRadius: nodeItem && nodeItem.style ? nodeItem.style.backRadius : 4.    

    function getNearestSocket(group, pos, containeritem) {
        //pos是grid层面的pos
        var paramsgroup = (group == ParamGroup.InputObject) ? inputobjparams : outputobjparams;
        var min_dist = 10000000
        var nearest_obj = null
        for (var i = 0; i < paramsgroup.count; i++) {
            var instZObjSock = paramsgroup.itemAt(i)
            if (instZObjSock.visible) {
                var sockpos = containeritem.mapFromGlobal(instZObjSock.mapToGlobal(Qt.point(instZObjSock.width/2, instZObjSock.height/2)))
                let distance = Math.sqrt(Math.pow(sockpos.x - pos.x, 2) + Math.pow(sockpos.y - pos.y, 2));
                if (distance < min_dist) {
                    min_dist = distance
                    nearest_obj = instZObjSock
                }
            }
        }
        return nearest_obj
    }

    function getSocketObject(paramName, group) {
        if (group == ParamGroup.InputObject) {
            var idx = nodeItem.node.params.indexFromName(paramName, true);
            var instZObjSock = inputobjparams.itemAt(idx)
            return instZObjSock;
        }
        else if (group == ParamGroup.InputPrimitive) {

        }
        else if (group == ParamGroup.OutputPrimitive) {

        }
        else if (group == ParamGroup.OutputObject) {
            var idx = nodeItem.node.params.indexFromName(paramName, false);
            var instZObjSock = outputobjparams.itemAt(idx)
            return instZObjSock
        }
    }

    onIsviewChanged: function() {
        var graphM = nodeItem.graph.model;
        var idx = nodeItem.node.index;
        graphM.setData(idx, nodeItem.isview, Model.ROLE_NODE_ISVIEW)
        // console.log("onIsviewChanged: " + nodeItem.isview)
    }

    onIsbypassChanged: function() {
        var graphM = nodeItem.graph.model;
        var idx = nodeItem.node.index;
        graphM.setData(idx, nodeItem.isbypass, Model.ROLE_NODE_BYPASS)
        //console.log("onIsbypassChanged: " + nodeItem.isbypass)
    }

    onDataChanged: function(data, role) {
        //console.log(nodeItem.node.label + " onDataChanged: " + data + ", role = " + role)
        if (role == Model.ROLE_NODE_ISVIEW) {
            //console.log("data = " + data)
            nodeItem.isview = data
        }
        if (role == Model.ROLE_NODE_BYPASS) {
            nodeItem.isbypass = data
        }
        if (role == Model.ROLE_OBJPOS) {
            nodeItem.x = data.x
            nodeItem.y = data.y           
        }
        if (role == Model.ROLE_NODE_RUN_STATE) {
            nodeItem.nodestatus = data
        }
    }

    //event:
    onNodeDoubleClicked: function(node, pos) {
        var graphM = nodeItem.graph.model;
        var idx = nodeItem.node.index;
        var nodetype = graphM.data(idx, Model.ROLE_NODETYPE);       //ROLE_OBJPOS
        if (nodetype >= 5) {
            //子图
            var graph_path = graphM.path()
            var nodename = graphM.data(idx, Model.ROLE_NODE_NAME)   //ROLE_NODE_NAME
            graph_path.push(nodename)
            stack_main_graphview.jumpTo(graph_path);
        } else {
            var pos2 = nodename_editor.mapFromItem(nodeItem, pos)
            if (pos2.x > 0 && pos2.y > 0 &&
                pos2.x < nodename_editor.width &&
                pos2.y < nodename_editor.height) {
                nodename_editor.isEditing = true
            }
        }
    }

    EditableText {
        id: detach_name_editor
        anchors.right: nodeItem.left
        y: right_status_group.y + right_status_group.height / 2 - height / 2
        //anchors.verticalCenter: right_status_group.verticalCenter
        //anchors.verticalCenter: mainmain_layout.verticalCenter
        text: nodeItem.node.label

        onTextChanged: {
            nodeItem.node.label = text
        }
    }

    Image {
        id: errormark
        visible: nodeItem.nodestatus == NodeStatus.RunError
        width: 64
        height: 64
        z: -10
        source: "qrc:/icons/node/error.svg"
        anchors.right: nodeItem.left
        y: right_status_group.y + right_status_group.height / 2 - height / 2
    }     

    ColumnLayout {
        id: mainmain_layout
        spacing: 1

        RowLayout {
            id: inputsocks_layout

            Item {
                Layout.fillWidth: true
            }

            Repeater{
                id: inputobjparams
                model: nodeItem.node.params

                delegate: 
                    ZObjSocket {
                        id: input_obj_socket
                        required property string name
                        required property string nodename
                        required property int group
                        required property color socket_color

                        socket_name: name
                        socket_group: group
                        bg_color: socket_color

                        visible: group == ParamGroup.InputObject

                        onSocketClicked: function() {
                            var centerpos = Qt.point(input_obj_socket.width / 2, input_obj_socket.height / 2)
                            var globalPosition = input_obj_socket.mapToGlobal(centerpos)
                            nodeItem.socketClicked(nodeItem, ParamGroup.InputObject, socket_name, globalPosition)
                        }
                    }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        RowLayout {
            //为了使节点框内的布局居中，所以在外面再套一个横向布局
            id: title_prims_layout

            Item {
                Layout.fillWidth: true
            }        

            Item {
                id: main_item
                implicitWidth: main_layout.implicitWidth
                implicitHeight: main_layout.implicitHeight

                Loader {
                    id: delegateLoader
                    anchors.fill: parent
                    source: "qrc:/QuickQanava/RectSolidShadowBackground.qml"

                    onItemChanged: {
                        if (item)
                            item.style = nodeItem.style
                    }
                }

                ColumnLayout {
                    id: main_layout
                    anchors.margins: 0
                    spacing: 0

                    Item {
                        id: nodeheader
                        implicitWidth: nodeheader_layout.implicitWidth
                        implicitHeight: nodeheader_layout.implicitHeight
                        Layout.fillWidth: true

                        /*以下两个rect各选一，视乎数值参数是否隐藏*/
                        ZRoundRect {
                            id: roundrectheader
                            anchors.fill: parent
                            bgcolor: "#5F5F5F"
                            radius: nodeItem.backRadius
                            visible: nodebody.visible
                        }

                        Rectangle {
                            id: rectheader
                            anchors.fill: parent
                            color: "#5F5F5F"
                            radius: nodeItem.backRadius
                            visible: !nodebody.visible
                        }

                        RowLayout {
                            id: nodeheader_layout
                            Layout.fillWidth: true
                            spacing: 1

                            RowLayout {
                                id: nodenamelayout
                                Layout.fillWidth: true

                                Rectangle {
                                    width: 18
                                    Layout.fillHeight: true
                                    color: "transparent"
                                }

                                EditableText {
                                    id: nodename_editor
                                    Layout.fillWidth: true
                                    //horizontalAlignment: Text.AlignHCenter
                                    text: nodeItem.node.label
                                    handle_mouseevent: false

                                    onTextChanged: {
                                        nodeItem.node.label = text
                                    }
                                }

                                Image {
                                    id: nodeicon
                                    width: 20
                                    height: 20
                                    //source: "data:image/svg+xml;utf8," + ""
                                }    

                                Rectangle {
                                    width: 18
                                    Layout.fillHeight: true
                                    color: "transparent"
                                }
                            }

                            StatusBtnGroup {
                                id: right_status_group
                                radius: nodeItem.backRadius
                                round_last_btn: !nodebody.visible
                                z: -50
                            }
                        }
                    }

                    Item {
                        id: nodebody
                        implicitWidth: nodebody_layout.implicitWidth
                        implicitHeight: nodebody_layout.implicitHeight
                        Layout.fillWidth: true
                        visible: nodeItem.node.params.showPrimSocks;

                        ZRoundRect {
                            anchors.fill: parent
                            bgcolor: "#303030"
                            radius: nodeItem.backRadius
                            top_radius: false
                        }

                        ColumnLayout {
                            id: nodebody_layout
                            anchors.fill: parent

                            Item {
                                id: just_top_margin
                                height: 5
                            }

                            Repeater{
                                id: inputprimparams
                                model: nodeItem.node.params

                                delegate:
                                    Text {
                                        required property string name
                                        required property int group
                                        required property bool socket_visible
                                        readonly property int hmargin: 10

                                        visible: group == ParamGroup.InputPrimitive && socket_visible

                                        color: "white"
                                        text: name
                                        Layout.alignment: Qt.AlignLeft
                                        Layout.leftMargin: hmargin

                                        Rectangle {
                                            id: input_prim_socket
                                            height: parent.height * 0.8
                                            width: height
                                            radius: height / 2
                                            color: "#CCA44E"
                                            x: -parent.hmargin - width/2.
                                            anchors.verticalCenter: parent.verticalCenter

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: function() {
                                                    var centerpos = Qt.point(input_prim_socket.width / 2, input_prim_socket.height / 2)
                                                    var globalPosition = input_prim_socket.mapToGlobal(centerpos)
                                                    nodeItem.socketClicked(nodeItem, ParamGroup.InputPrimitive, name, globalPosition)
                                                }
                                            }
                                        }
                                    }
                            }

                            Repeater {
                                id: outputprimparams
                                model: nodeItem.node.params

                                delegate:
                                    Text {
                                        required property string name
                                        required property int group
                                        required property bool socket_visible
                                        readonly property int hmargin: 10

                                        visible: group == ParamGroup.OutputPrimitive && socket_visible

                                        color: "white"
                                        text: name
                                        Layout.alignment: Qt.AlignRight
                                        Layout.rightMargin: 10

                                        Rectangle {
                                            id: output_prim_socket
                                            height: parent.height * 0.8
                                            width: height
                                            radius: height / 2
                                            color: "#CCA44E"
                                            x: parent.width + parent.hmargin - width/2.
                                            anchors.verticalCenter: parent.verticalCenter
                                            
                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: function() {
                                                    var centerpos = Qt.point(output_prim_socket.width / 2, output_prim_socket.height / 2)
                                                    var globalPosition = output_prim_socket.mapToGlobal(centerpos)
                                                    nodeItem.socketClicked(nodeItem, ParamGroup.OutputPrimitive, name, globalPosition)
                                                }
                                            }
                                        }
                                    }
                            }

                            Item {
                                id: just_bottom_margin
                                height: 5
                            }
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }

        Rectangle {
            id: dirtymark
            color: "yellow"
            height: 2
            width: 48
            visible: nodeItem.nodestatus != NodeStatus.RunSucceed
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            id: outputsocks_layout

            Item {
                Layout.fillWidth: true
            }

            Repeater{
                id: outputobjparams
                model: nodeItem.node.params

                delegate: ZObjSocket {
                    id: output_obj_socket
                    required property string nodename
                    required property string name
                    required property int group
                    required property color socket_color

                    socket_name: name
                    socket_group: group
                    bg_color: socket_color
                    visible: group == ParamGroup.OutputObject  //对应代码NodeDataGroup枚举值
                    onSocketClicked: function() {
                        var centerpos = Qt.point(output_obj_socket.width / 2, output_obj_socket.height / 2)
                        var globalPosition = output_obj_socket.mapToGlobal(centerpos)
                        nodeItem.socketClicked(nodeItem, ParamGroup.OutputObject, socket_name, globalPosition)
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Component.onCompleted: {
        //初始化基本数据:
        var graphM = nodeItem.graph.model;
        var idx = nodeItem.node.index;
        var nodename = graphM.data(idx, Model.ROLE_NODE_NAME)   //ROLE_NODE_NAME
        var pos = graphM.data(idx, Model.ROLE_OBJPOS);       //ROLE_OBJPOS
        nodeItem.x = pos.x
        nodeItem.y = pos.y
        nodeItem.isview = graphM.data(idx, Model.ROLE_NODE_ISVIEW)

        var uistyle = graphM.data(idx, Model.ROLE_NODE_UISTYLE)
        if (uistyle["icon"] != "") {
            nodeicon.visible = true
            detach_name_editor.visible = true
            nodename_editor.visible = false
            nodeicon.source = "data:image/svg+xml;utf8," + uistyle["icon"]
        }
        else {
            nodeicon.visible = false
            detach_name_editor.visible = false
            nodename_editor.visible = true
        }
        //console.log("ui.icon = " + uistyle["icon"])
        if (uistyle["background"] != "") {
            roundrectheader.bgcolor = rectheader.color = uistyle["background"];
        }
    }
}