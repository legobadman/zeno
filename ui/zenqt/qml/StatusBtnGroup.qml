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
    property bool round_last_btn: true
    property bool isview: false
    property bool isbypass: false

    implicitWidth: 2 * comp.side
    implicitHeight: fixheight

    MouseArea{
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        // onExited: {
        //     if(!groupContainsMouse(mouseX, mouseY)) {
        //         statusImgGroup.visible = false
        //     }
        // }

        StatusBtn {
            id: bypass_btn
            basefillcolor: "#FFBD21"
            height: fixheight
            xoffset: comp.xoffset
            side: comp.side
            round_last_btn: false
            is_left_or_right: false
            property alias checked: comp.isbypass

            onHoveredChanged: {
                if (bypass_btn.hovered) {
                    statusImgGroup.visible = true;
                }
            }

            onCheckedChanged: {
                statusImgGroup.visible = false;
            }
        }

        /*
        StatusBtnSeperator {
            xoffset: comp.xoffset
            x: comp.side
        }*/

        StatusBtn {
            id: view_btn
            basefillcolor: "#30BDD4"
            height: fixheight
            xoffset: comp.xoffset
            side: comp.side
            radius: comp.radius
            round_last_btn: comp.round_last_btn
            is_left_or_right: true
            x: comp.side + 1
            property alias checked: comp.isview

            onHoveredChanged: {
                if (view_btn.hovered) {
                    statusImgGroup.visible = true;
                }
            }

            onCheckedChanged: {
                statusImgGroup.visible = false;
            }
        }
    }

    Item {
        id: statusImgGroup
        visible: false
        implicitWidth: imggroup_layout.implicitWidth
        implicitHeight: imggroup_layout.implicitHeight
        y: bypass_btn.y - imgByPass.height
        z: -100

        Rectangle {
            anchors.fill: parent
            // color: "red"
            color: "transparent"
        }

        ColumnLayout {
            id: imggroup_layout

            Rectangle{
                id: img_group
                width:childrenRect.width
                height:childrenRect.height
                color: "transparent"
                
                StatusImgBtn{
                    id: imgByPass
                    x: bypass_btn.x + comp.xoffset - 2
                    source: "qrc:/icons/MUTE_dark.svg"
                    source_on: "qrc:/icons/MUTE_light.svg"
                    property alias checked: comp.isbypass
                    property alias hovered: bypass_btn.hovered
                }
                StatusImgBtn{
                    id: imgView
                    x: imgByPass.x + imgByPass.width - imgByPass.xoffset - 2
                    source: "qrc:/icons/VIEW_dark.svg"
                    source_on: "qrc:/icons/VIEW_light.svg"
                    property alias checked: comp.isview
                    property alias hovered: view_btn.hovered
                }
            }
            Rectangle{
                id: rc_placeholder
                width: img_group.width
                height: 64
                color: "transparent"
            }

        }

        MouseArea {
            id: mouse_area_imggroup
            hoverEnabled: true
            anchors.fill: parent

            function isbypass(mousepos) {
                if (mousepos.x < imgView.x && mousepos.y < imgByPass.y + imgByPass.height) {
                    return true
                }
                else {
                    return false
                }
            }

            onPositionChanged: {
                if (isbypass(mouse)) {
                    imgByPass.hovered = true
                    imgView.hovered = false
                }
                else {
                    imgView.hovered = true
                    imgByPass.hovered = false
                }
                // console.log("mouse.x = " + mouse.x)
                // console.log("parent.x = " + statusImgGroup.x)
                // console.log("imgByPass.x = " + imgByPass.x)
                // console.log("imgView.x = " + imgView.x)
            }

            onExited: {
                imgByPass.hovered = false
                imgView.hovered = false
                statusImgGroup.visible = false
            }

            onPressed: {
                if (isbypass(mouse)) {
                    imgByPass.checked = !imgByPass.checked
                }
                else{
                    imgView.checked = !imgView.checked
                }
                statusImgGroup.visible = false
            }
        }
    }



    function groupContainsMouse(x, y){
        //console.log("mouse.x,y = " + x + "," + y)
        var under_imgs = false;
        // if (x < 100 && y < 48) {
        //     under_imgs = true;
        // }
        console.log("mouse_area_imggroup.containsMouse: " + mouse_area_imggroup.containsMouse)

        return bypass_btn.mouseAreaAlias.containsMouse || view_btn.mouseAreaAlias.containsMouse || mouse_area_imggroup.containsMouse
            // imgByPass.containsMouse || imgView.containsMouse || under_imgs
    }
}