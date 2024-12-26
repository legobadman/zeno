import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import zeno.enum 1.0

Item {
    id: root
    property var graphModel: undefined
    property var graphView: undefined

    Repeater {
        model: root.graphModel.getLinkModel()
        
        /* 用于debug位置
        delegate: Text {
            required property var fromParam
            required property var toParam

            text: '(' + fromParam[0] + ',' + fromParam[1] + ') -> (' + toParam[0] + ',' + toParam[1] + ')';
            color: root.graphView.resizeHandlerColor

            Component.onCompleted: {
                x = Qt.binding(function() {
                    console.log("Qt.binding x")
                    var out_nodeitem = root.graphView.graph.getNode(fromParam[0]).item
                    var out_sockitem = out_nodeitem.getSocketObject(fromParam[1], ParamGroup.OutputObject)
                    var outsock_pos_in_grid = root.graphView.containerItem.mapFromGlobal(out_sockitem.mapToGlobal(Qt.point(out_sockitem.width/2, out_sockitem.height/2)))
                    
                    out_nodeitem.x
                    out_nodeitem.y
                    out_nodeitem.height
                    
                    return outsock_pos_in_grid.x;
                })

                y = Qt.binding(function() {
                    console.log("Qt.binding y")
                    var out_nodeitem = root.graphView.graph.getNode(fromParam[0]).item
                    var out_sockitem = out_nodeitem.getSocketObject(fromParam[1], ParamGroup.OutputObject)
                    var outsock_pos_in_grid = root.graphView.containerItem.mapFromGlobal(out_sockitem.mapToGlobal(Qt.point(out_sockitem.width/2, out_sockitem.height/2)))
                    
                    out_nodeitem.x
                    out_nodeitem.y
                    out_nodeitem.height

                    return outsock_pos_in_grid.y;
                })

                
                var out_nodeitem = root.graphView.graph.getNode(fromParam[0]).item
                var out_sockitem = out_nodeitem.getSocketObject(fromParam[1], ParamGroup.OutputObject)
                var outsock_pos_in_grid = root.graphView.containerItem.mapFromItem(root.graphView, out_sockitem.mapToGlobal(Qt.point(out_sockitem.width/2, out_sockitem.height/2)))
                console.log("outsock_pos_in_grid: " + outsock_pos_in_grid);

                var in_nodeitem = root.graphView.graph.getNode(toParam[0]).item
                var in_sockitem = in_nodeitem.getSocketObject(toParam[1], ParamGroup.InputObject);
                var insock_pos_in_grid = root.graphView.containerItem.mapFromItem(root.graphView, in_sockitem.mapToGlobal(Qt.point(in_sockitem.width/2, in_sockitem.height/2)))
                console.log("insock_pos_in_grid: " + insock_pos_in_grid);
                
            }
        }
        */
        
        delegate: Edge {
            required property var fromParam
            required property var toParam

            visible: true
            point1x: 0
            point1y: 0
            point2x: 0
            point2y: 0
            color: "#4E9EF4"

            Component.onCompleted: {
                point1x = Qt.binding(function() {
                    var out_nodeitem = root.graphView.graph.getNode(fromParam[0]).item
                    var out_sockitem = out_nodeitem.getSocketObject(fromParam[1], ParamGroup.OutputObject)
                    var outsock_pos_in_grid = root.graphView.containerItem.mapFromGlobal(out_sockitem.mapToGlobal(Qt.point(out_sockitem.width/2, out_sockitem.height/2)))
                    console.log("outsock_pos_in_grid: " + outsock_pos_in_grid);

                    out_nodeitem.x
                    out_nodeitem.y
                    out_nodeitem.width

                    return outsock_pos_in_grid.x
                })

                point1y = Qt.binding(function() {
                    var out_nodeitem = root.graphView.graph.getNode(fromParam[0]).item
                    var out_sockitem = out_nodeitem.getSocketObject(fromParam[1], ParamGroup.OutputObject)
                    var outsock_pos_in_grid = root.graphView.containerItem.mapFromGlobal(out_sockitem.mapToGlobal(Qt.point(out_sockitem.width/2, out_sockitem.height/2)))
                    
                    out_nodeitem.x
                    out_nodeitem.y
                    out_nodeitem.height

                    return outsock_pos_in_grid.y
                })

                point2x = Qt.binding(function() {
                    var in_nodeitem = root.graphView.graph.getNode(toParam[0]).item
                    var in_sockitem = in_nodeitem.getSocketObject(toParam[1], ParamGroup.InputObject);
                    var insock_pos_in_grid = root.graphView.containerItem.mapFromGlobal(in_sockitem.mapToGlobal(Qt.point(in_sockitem.width/2, in_sockitem.height/2)))

                    in_nodeitem.x
                    in_nodeitem.y
                    in_nodeitem.width
                    
                    return insock_pos_in_grid.x
                })

                point2y = Qt.binding(function() {
                    var in_nodeitem = root.graphView.graph.getNode(toParam[0]).item
                    var in_sockitem = in_nodeitem.getSocketObject(toParam[1], ParamGroup.InputObject);
                    var insock_pos_in_grid = root.graphView.containerItem.mapFromGlobal(in_sockitem.mapToGlobal(Qt.point(in_sockitem.width/2, in_sockitem.height/2)))

                    in_nodeitem.x
                    in_nodeitem.y
                    in_nodeitem.height

                    return insock_pos_in_grid.y
                })
            }
        }
    }

    Component.onCompleted: {
        
    }
}