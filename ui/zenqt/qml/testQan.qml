import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan

Qan.GraphView {
    anchors.fill: parent
    id: graphView
    navigable   : true
    PinchHandler {
        target: null
        onActiveScaleChanged: {
            console.error('centroid.position=' + centroid.position)
            console.error('activeScale=' + activeScale)
            var p = centroid.position
            var f = activeScale > 1.0 ? 1. : -1.
            navigable.zoomOn(p, navigable.zoom + (f * 0.03))
        }
    }
    graph: Qan.Graph {
        id: graph
        Component.onCompleted: {
        }
    }
}  // Qan.GraphView
