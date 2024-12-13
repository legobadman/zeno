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

    readonly property real backRadius: nodeItem && nodeItem.style ? nodeItem.style.backRadius : 4.    

    /*
    Loader {
        id: delegateLoader
        anchors.fill: parent
        source: {
            if (!nodeItem || !nodeItem.style)     // Defaul to solid no effect with unconfigured nodes
                return "qrc:/QuickQanava/RectSolidBackground.qml";
            switch (nodeItem.style.fillType) {  // Otherwise, select the delegate according to current style configuration
                case Qan.NodeStyle.FillSolid:
                switch (nodeItem.style.effectType) {
                    case Qan.NodeStyle.EffectNone:   return "qrc:/QuickQanava/RectSolidBackground.qml";
                    case Qan.NodeStyle.EffectShadow: return "qrc:/QuickQanava/RectSolidShadowBackground.qml";
                    case Qan.NodeStyle.EffectGlow:   return "qrc:/QuickQanava/RectSolidGlowBackground.qml";
                }
                break;
                case Qan.NodeStyle.FillGradient:
                    switch (nodeItem.style.effectType) {
                    case Qan.NodeStyle.EffectNone:   return "qrc:/QuickQanava/RectGradientBackground.qml";
                    case Qan.NodeStyle.EffectShadow: return "qrc:/QuickQanava/RectGradientShadowBackground.qml";
                    case Qan.NodeStyle.EffectGlow:   return "qrc:/QuickQanava/RectGradientGlowBackground.qml";
                }
                break;
            } // case fillType
        }
        onItemChanged: {
            if (item)
                item.style = nodeItem.style
        }
    }
    */

    ColumnLayout {
        id: mainmain_layout
        implicitWidth: main_item.implicitWidth;
        implicitHeight: main_item.implicitHeight + 32;
        spacing: 1

        Rectangle {
            width: 64
            height: 32
            color: "red"
        }

        Item {
            id: main_item
            implicitWidth: main_layout.implicitWidth// + 2 * backRadius
            implicitHeight: main_layout.implicitHeight;// + 2 * backRadius

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
                anchors.margins: 0;//backRadius / 2.

                RowLayout {
                    id: nodeheader
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
                        }

                        Rectangle {
                            width: 18
                            Layout.fillHeight: true
                            color: "transparent"
                        }
                    }

                    StatusBtnGroup {
                        radius: nodeItem.backRadius
                    }
                }
            }
        }

        Rectangle {
            width: 64
            height: 32
            color: "green"
        }
    }
}