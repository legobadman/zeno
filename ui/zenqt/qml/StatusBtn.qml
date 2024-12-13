import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Shapes 1.15


Item {
    id: comp
    property var basefillcolor
    property alias color: path.fillColor
    property alias mouseAreaAlias: mouseArea

    property int xoffset: 22
    property int side: 35

    implicitWidth: xoffset + side
    implicitHeight: parent.height

    // layer.enabled: true
    // layer.samples: 64

    signal statusChanged(bool status)

    Shape {
        id: sp
        anchors.fill: parent
        antialiasing: true
        smooth: true

        containsMode: Shape.FillContains
        property bool clicked: false
        ShapePath {
            id: path
            strokeColor: "transparent"
            strokeWidth: 0

            fillColor: sp.clicked ? basefillcolor : "#2E313A"
            capStyle: ShapePath.RoundCap
            
            property int joinStyleIndex: 0

            property variant styles: [
                ShapePath.BevelJoin,
                ShapePath.MiterJoin,
                ShapePath.RoundJoin
            ]

            property bool containsMouse: {
                //todo
                return true;
            }

            joinStyle: styles[joinStyleIndex]

            startX: comp.xoffset
            startY: 0
            PathLine { x: comp.xoffset + comp.side; y: 0 }
            PathLine { x: comp.side; y: comp.height}
            PathLine { x: 0; y: comp.height }
        }
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            containmentMask: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton
            onClicked: {
                doClick()
            }
            onEntered:{
                path.fillColor = basefillcolor
                statusChanged(true)
            }
            onExited:{
                if(!parent.clicked){
                    path.fillColor = "#2E313A"
                    statusChanged(false)
                }
            }
            function doClick(){
                parent.clicked = !parent.clicked
                if(parent.clicked){
                    path.fillColor = basefillcolor
                    statusChanged(true)
                }
                else{
                    path.fillColor = "#2E313A"
                    statusChanged(false)
                }
            }
        }
    }
}


