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


Qan.NodeItem {
    id: nodeItem
    //width: 110; height: 60
    //x: 15;      y: 15

    // 自动适应子布局的尺寸
    implicitWidth: mainmain_layout.implicitWidth
    implicitHeight: mainmain_layout.implicitHeight;
    resizable: false
    //clip: true        //设置会导致选框不出现

    readonly property real backRadius: nodeItem && nodeItem.style ? nodeItem.style.backRadius : 4.    

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
                    Rectangle {
                        id: inputobj_sock
                        required property string name
                        required property int group
                        required property color socket_color

                        height: childrenRect.height
                        width: childrenRect.width
                        color: socket_color
                        visible: group == 0

                        Text {
                            color: "black"
                            text: inputobj_sock.name
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
                        id:nodeheader
                        implicitWidth: nodeheader_layout.implicitWidth
                        implicitHeight: nodeheader_layout.implicitHeight
                        Layout.fillWidth: true

                        /*以下两个rect各选一，视乎数值参数是否隐藏*/
                        ZRoundRect {
                            anchors.fill: parent
                            bgcolor: "#5A9CE0"
                            radius: nodeItem.backRadius
                            visible: nodebody.visible
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: "#5A9CE0"
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

                                Label {
                                    Layout.fillWidth: true
                                    horizontalAlignment: Text.AlignHCenter
                                    text: nodeItem.node.label
                                    font.bold: true
                                    color: "white"
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

                                        visible: group == 1 && socket_visible

                                        color: "white"
                                        text: name
                                        Layout.alignment: Qt.AlignLeft
                                        Layout.leftMargin: hmargin

                                        Rectangle {
                                            height: parent.height * 0.8
                                            width: height
                                            radius: height / 2
                                            color: "#CCA44E"
                                            x: -parent.hmargin - width/2.
                                            anchors.verticalCenter: parent.verticalCenter
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

                                        visible: group == 3 && socket_visible

                                        color: "white"
                                        text: name
                                        Layout.alignment: Qt.AlignRight
                                        Layout.rightMargin: 10

                                        Rectangle {
                                            height: parent.height * 0.8
                                            width: height
                                            radius: height / 2
                                            color: "#CCA44E"
                                            x: parent.width + parent.hmargin - width/2.
                                            anchors.verticalCenter: parent.verticalCenter
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

        RowLayout {
            id: outputsocks_layout

            Item {
                Layout.fillWidth: true
            }

            Repeater{
                id: outputobjparams
                model: nodeItem.node.params

                delegate: Rectangle {
                        id: outputobj_sock
                        required property string name
                        required property int group
                        required property color socket_color

                        height: childrenRect.height
                        width: childrenRect.width
                        color: socket_color
                        visible: group == 2  //对应代码NodeDataGroup枚举值

                        Text {
                            color: "black"
                            text: outputobj_sock.name
                        }
                    }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}