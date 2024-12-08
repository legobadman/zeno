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
import "./container/TabView"


Zen.GraphsTotalView {
    id: totalview
    visible: true
    width: 1280; height: 720
    //anchors.fill: parent
    graphsMgr: Zen.GraphsManager {
        id: graphs
    }

    StackLayout {
        id: welcomepage_or_editor
        currentIndex: 0
        anchors.fill: parent

        WelcomePage {
            Layout.fillHeight: true
            Layout.fillWidth: true
            graphs: totalview
        }

        SplitView {
            id: mainLayout
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
                Layout.fillWidth: true;
                Layout.fillHeight: true
                currentIndex: 1

                Rectangle {
                    width: 200
                    implicitWidth: 200
                    Layout.maximumWidth: 400
                    color: "#EEEEEE"
                    Layout.fillHeight: true

                    // 1.定义delegate，内嵌三个Text对象来展示Model定义的ListElement的三个role
                    Component {
                        id: phoneDelegate
                        Item {
                            id: wrapper
                            width: parent.width
                            height: 30
                                
                            // 实现了鼠标点选高亮的效果
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: wrapper.ListView.view.currentIndex = index
                            }
                                
                            // 内嵌三个Text对象，水平布局
                            RowLayout {
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 8

                                Text { 
                                    id: col1;
                                    text: name;
                                    // 是否是当前条目
                                    color: wrapper.ListView.isCurrentItem ? "red" : "black"
                                    font.pixelSize: wrapper.ListView.isCurrentItem ? 22 : 18
                                    Layout.preferredWidth: 120
                                }
                                    
                                Text { 
                                    text: cost; 
                                    color: wrapper.ListView.isCurrentItem ? "red" : "black"
                                    font.pixelSize: wrapper.ListView.isCurrentItem ? 22 : 18
                                    Layout.preferredWidth: 80
                                }
                                    
                                Text { 
                                    text: manufacturer; 
                                    color: wrapper.ListView.isCurrentItem ? "red" : "black"
                                    font.pixelSize: wrapper.ListView.isCurrentItem ? 22 : 18
                                    Layout.fillWidth: true
                                }
                            }
                        }
                    } // phoneDelegate-END
                        
                    // 2.定义ListView
                    ListView {
                        id: listView
                        anchors.fill: parent

                        // 使用先前设置的delegate
                        delegate: phoneDelegate
                            
                        // 3.ListModel专门定义列表数据的，它内部维护一个 ListElement 的列表。
                        model: ListModel {
                            id: phoneModel

                            // 一个 ListElement 对象就代表一条数据
                            ListElement{
                                name: "iPhone 3GS"
                                cost: "1000"
                                manufacturer: "Apple"
                            }
                            ListElement{
                                name: "iPhone 4"
                                cost: "1800"
                                manufacturer: "Apple"
                            }            
                            ListElement{
                                name: "iPhone 4S"
                                cost: "2300"
                                manufacturer: "Apple"
                            } 
                            ListElement{
                                name: "iPhone 5"
                                cost: "4900"
                                manufacturer: "Apple"
                            }                                                
                        }

                        // 背景高亮
                        focus: true
                        highlight: Rectangle{
                            color: "lightblue"
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
                    id: bar
                    TabButton {
                        text: qsTr("main")
                        width: implicitWidth
                    }
                    CustomTabButton {
                        text: qsTr("Asset1")
                        width: implicitWidth
                    }
                    CustomTabButton {
                        text: qsTr("Asset2")
                        width: implicitWidth
                    }

                }

                StackLayout {
                    width: parent.width
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: bar.bottom
                    anchors.bottom: parent.bottom
                    currentIndex: bar.currentIndex
                    Rectangle {
                        id: homeTab
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "red"
                    }
                    Item { 
                        Loader { anchors.fill: parent; source: "qrc:/zenographview.qml"}
                    }
                    Rectangle {
                        id: activityTab
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "blue"
                    }
                }
            }
        }
    }

    onModelInited: function() {
        welcomepage_or_editor.currentIndex = 1
    }

    onFileClosed: function() {
        welcomepage_or_editor.currentIndex = 0
    }
}

