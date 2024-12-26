import QtQuick 2.12
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4


Item {
    id: root
    property var graphModel: undefined
    property var graphView: undefined

    Repeater {
        model: root.graphModel.getLinkModel()
        delegate: Text {
            required property var fromParam
            required property var toParam

            text: '(' + fromParam[0] + ',' + fromParam[1] + ') -> (' + toParam[0] + ',' + toParam[1] + ')';
            color: root.graphView.resizeHandlerColor

            Component.onCompleted: {
                var qannode = root.graphView.graph.getNode(fromParam[0]);
                if (qannode === null) {
                    console.log("cannot get qannode");
                }
                var nodeitem = qannode.item
                console.log("pos:" + nodeitem.x + "," + nodeitem.y + "  width=" + nodeitem.width + ",height=" + nodeitem.height);
                console.log("backRadius: " + nodeitem.backRadius)
                var wtf = nodeitem.findChild("input_obj_socket");
                console.log("wtf = " + wtf);
            }
        }

        // delegate: Edge {
        //     required property var fromParam
        //     required property var toParam


        // }
    }

    Component.onCompleted: {
        
    }
}