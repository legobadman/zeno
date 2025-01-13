﻿import QtQuick 2.15
import QtQuick.Shapes 1.15
import zeno.enum 1.0

Shape {
    id: root

    property real point1x       //point1一般是临时边的发起点
    property real point1y
    property real point2x
    property real point2y
    property bool isFromInput: false
    property int p1_group: ParamGroup.InputObject       //point1的group

    property var tempedge_from_sock: undefined      //临时边起始的socket对象，可以是input或者output
    property var tempedge_to_sock: undefined      //吸附时记录的目标socket对象,只有临时边会用

    property bool isSelected: false

    property string nodeId
    property string paramName
    property bool isMatch: false

    property alias thickness: path.strokeWidth

    property color color: Qt.rgba(192/255, 36/255, 36/255, 0.6)
    property color color_hover: "#FFFFFF"
    property color color_selected: Qt.rgba(250/255, 100/255, 0, 1.0)


    // BUG: edgeArea is destroyed before path, need to test if not null to avoid warnings
    //readonly property bool containsMouse: edgeArea && edgeArea.containsMouse

    signal clicked
    signal clicked_outof_curve

    x: Math.min(point1x, point2x)
    y: Math.min(point1y, point2y)

    width: Math.abs(point2x - point1x)
    height: Math.abs(point2y - point1y)

    property real startX: (point1x > point2x) ? width : 0
    property real startY: (point1y > point2y) ? height : 0
    property real endX: (point1x > point2x) ? 0 : width
    property real endY: (point1y > point2y) ? 0 : height

    // cause rendering artifacts when enabled (and don't support hot reload really well)
    vendorExtensionsEnabled: false

    Rectangle {
        //用于调试点击测试区域
        anchors.fill: parent
        color: "transparent"
        // border.color: "red"
        // border.width: 2
    }

    ShapePath {
        id: path
        startX: root.startX
        startY: root.startY
        fillColor: "transparent"
        strokeColor: root.isSelected ? root.color_selected : root.color
        //strokeStyle: edge !== undefined && ((edge.src !== undefined && edge.src.isOutput) || edge.dst === undefined) ? ShapePath.SolidLine : ShapePath.DashLine
        strokeStyle: ShapePath.SolidLine
        strokeWidth: 4
        // final visual width of this path (never below 1)
        readonly property real visualWidth: Math.max(strokeWidth, 1)
        dashPattern: [6/visualWidth, 4/visualWidth]
        capStyle: ShapePath.RoundCap

        PathCubic {
            id: cubic
            property real ctrlPtDist: 60
            x: root.endX
            y: root.endY
            control1X: {
                if (root.p1_group == ParamGroup.InputObject) {
                    return path.startX;
                }
                else if (root.p1_group == ParamGroup.InputPrimitive){
                    return path.startX - ctrlPtDist;
                }
                else if (root.p1_group == ParamGroup.OutputObject){
                    return path.startX;
                }
                else if (root.p1_group == ParamGroup.OutputPrimitive){
                    return path.startX + ctrlPtDist;
                }
                return -1;
            }
            control1Y: {
                if (root.p1_group == ParamGroup.InputObject) {
                    return path.startY - ctrlPtDist;
                }
                else if (root.p1_group == ParamGroup.InputPrimitive){
                    return path.startY;
                }
                else if (root.p1_group == ParamGroup.OutputObject){
                    return path.startY + ctrlPtDist;
                }
                else if (root.p1_group == ParamGroup.OutputPrimitive){
                    return path.startY;
                }
                return -1;
            }
            control2X: {
                if (root.p1_group == ParamGroup.InputObject) {
                    return root.endX;
                }
                else if (root.p1_group == ParamGroup.InputPrimitive){
                    return root.endX + ctrlPtDist;
                }
                else if (root.p1_group == ParamGroup.OutputObject){
                    return root.endX;
                }
                else if (root.p1_group == ParamGroup.OutputPrimitive){
                    return root.endX - ctrlPtDist;
                }
                return -1;
            }
            control2Y: {
                if (root.p1_group == ParamGroup.InputObject) {
                    return root.endY + ctrlPtDist;
                }
                else if (root.p1_group == ParamGroup.InputPrimitive){
                    return root.endY;
                }
                else if (root.p1_group == ParamGroup.OutputObject){
                    return root.endY - ctrlPtDist;
                }
                else if (root.p1_group == ParamGroup.OutputPrimitive){
                    return root.endY;
                }
                return -1;
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: {
            const threshold = 50; // 点击检测的阈值
            //console.log("edge mouse on click")

            // 判断鼠标点击点是否接近曲线
            function isPointCloseToCurve(px, py) {
                const steps = 10; // 将曲线分成 100 个点
                for (let t = 0; t <= 1; t += 1 / steps) {
                    let bx = Math.pow(1 - t, 3) * root.startX +
                             3 * Math.pow(1 - t, 2) * t * cubic.control1X +
                             3 * (1 - t) * Math.pow(t, 2) * cubic.control2X +
                             Math.pow(t, 3) * root.endX;

                    let by = Math.pow(1 - t, 3) * root.startY +
                             3 * Math.pow(1 - t, 2) * t * cubic.control1Y +
                             3 * (1 - t) * Math.pow(t, 2) * cubic.control2Y +
                             Math.pow(t, 3) * root.endY;

                    // console.log("px = " + px + ", py = " + py)
                    // console.log("bx = " + bx + ", by = " + by)
                    let distance = Math.sqrt(Math.pow(px - bx, 2) + Math.pow(py - by, 2));
                    // console.log("distance: " + distance)
                    if (distance < threshold) {
                        return true;
                    }
                }
                return false;
            }

            if (isPointCloseToCurve(mouse.x, mouse.y)) {
                // console.log("曲线被点击！");
                root.clicked()
                mouse.accepted = true;
            } else {
                //console.log("点击区域未覆盖曲线");
                root.clicked_outof_curve()
                mouse.accepted = false;
            }
        }
    }
}