import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Shapes 1.6


Item {
    id: comp
    property int xoffset: 12
    property int side: 24
    property int fixheight: 48
    property real radius: 4

    implicitWidth: 2 * comp.side
    implicitHeight: fixheight

    MouseArea{
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onExited: {
            if(!groupContainsMouse())
                statusImgGroup.visible = false
        }

        StatusBtn {
            id: bypass_btn
            basefillcolor: "#FFBD21"
            height: fixheight
            xoffset: comp.xoffset
            side: comp.side
            property bool is_entered : false

            onStatusChanged: (status)=> {
                if (!bypass_btn.is_entered) {
                    bypass_btn.is_entered = true
                    statusImgGroup.visible = true
                    imgByPass.source = status ? "qrc:/icons/MUTE_light.svg" : "qrc:/icons/MUTE_dark.svg"
                    bypass_btn.is_entered = false
                }
            }
        }

        /*
        StatusBtnSeperator {
            xoffset: comp.xoffset
            x: comp.side
        }*/

        StatusRoundBtn {
            id: view_btn
            basefillcolor: "#30BDD4"
            height: fixheight
            xoffset: comp.xoffset
            side: comp.side
            radius: comp.radius
            x: comp.side + 1
            property bool is_entered : false

            onStatusChanged: (status)=> {
                if (!view_btn.is_entered) {
                    view_btn.is_entered = true
                    statusImgGroup.visible = true
                    imgView.source = status ? "qrc:/icons/VIEW_light.svg" : "qrc:/icons/VIEW_dark.svg"
                    view_btn.is_entered = false
                }
            }
        }
    }
    Rectangle{
        id: statusImgGroup
        anchors.bottom: mouseArea.top

        width:childrenRect.width
        height:childrenRect.height
        color: "transparent"
        visible: false

        StatusImgBtn{
            id: imgByPass
            x: bypass_btn.x + comp.xoffset - 2
            source: "qrc:/icons/MUTE_dark.svg"
            property bool is_entered : false

            onClickedSig: bypass_btn.mouseAreaAlias.doClick()
            onEnteredSig: {
                if (!imgByPass.is_entered) {
                    imgByPass.is_entered = true
                    bypass_btn.mouseAreaAlias.entered()
                    imgByPass.is_entered = false
                }
            }
            onExitedSig: {
                if (!imgByPass.is_entered) {
                    imgByPass.is_entered = true
                    bypass_btn.mouseAreaAlias.exited()
                    if(!groupContainsMouse())
                        statusImgGroup.visible = false
                    imgByPass.is_entered = false
                }
            }
        }
        StatusImgBtn{
            id: imgView
            x: view_btn.x + bypass_btn.width - 1
            source: "qrc:/icons/VIEW_dark.svg"
            property bool is_entered : false
            
            onClickedSig: view_btn.mouseAreaAlias.doClick()
            onEnteredSig: {
                if (!imgView.is_entered) {
                    imgView.is_entered = true;
                    view_btn.mouseAreaAlias.entered()
                    bypass_btn.mouseAreaAlias.exited()
                    imgView.is_entered = false;
                }
            }
            onExitedSig: {
                if (!imgView.is_entered) {
                    imgView.is_entered = true;
                    view_btn.mouseAreaAlias.exited()
                    if(!groupContainsMouse())
                        statusImgGroup.visible = false
                    imgView.is_entered = false;
                }
            }
        }
    }
    function groupContainsMouse(){
        return bypass_btn.mouseAreaAlias.containsMouse || view_btn.mouseAreaAlias.containsMouse ||
            imgByPass.containsMouse || imgView.containsMouse
    }
}