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

import QtQuick                   2.8
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import QtQuick.Shapes            1.0

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan
import Zeno 1.0 as Zen
import "./view"
import "./container/TabView"


Zen.GraphsTotalView {
    id: window
    visible: true
    width: 1280; height: 720
    //anchors.fill: parent
    graphsMgr: Zen.GraphsManager {
        id: graphs
    }

    // 定义全局对象，通过 editor 来访问
    Item {
        id: editor

        // 标签页逻辑控制器
        TabViewController { id: tab_ }
        property var tab: tab_ // 通过 editor.tab 来访问

        // 持久化存储
        Settings { 
            id: settings
            fileName: "./.settings_ui.ini" // 配置文件名


            property alias openPageList: tab_.openPageList
            property alias showPageIndex: tab_.showPageIndex
            property alias barIsLock: tab_.barIsLock

            property bool refresh: false // 用于刷新
            function save(){ // 手动刷新
                refresh=!refresh
            }
        }
    }

    SplitView {
        id: mainLayout
        spacing: 10
        anchors.fill: parent
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

        Rectangle {
            SplitView.preferredWidth: 340
            anchors.top: parent.top
            color: "#181818"

            TreeView {
                id: styledTreeView

                anchors.fill: parent
                anchors.margins: 1

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

        TabView_ { 
            id: tabEditor
            SplitView.fillWidth: true
        }
    }


    /*
    Pane { anchors.fill: parent }
    ColumnLayout {
        anchors.fill: parent
        TabBar {
            id: tabBar
            Layout.preferredWidth: 450; Layout.fillHeight: false
            TabButton { text: qsTr("Zeno Nodes") }
        }
        StackLayout {
            clip: true
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            Item { Loader { anchors.fill: parent; source: "qrc:/zenographview.qml"} }
        }
    }
    RowLayout {
        anchors.top: parent.top;    anchors.right: parent.right
        CheckBox {
            text: qsTr("Dark")
            checked: ApplicationWindow.contentItem.Material.theme === Material.Dark
            onClicked: ApplicationWindow.contentItem.Material.theme = checked ? Material.Dark : Material.Light
        }
    }
    */

    onModelInited: function() {
        console.log("model inited2.")
    }
}

