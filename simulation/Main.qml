import QtQuick
import QtQuick.Window

Window {
    width: 900
    height: 580
    visible: true
    title: qsTr("Hello World")
    color: "white"

    Rectangle {
        id: rect
        anchors.centerIn: parent
        border.color: "#aaa"
        border.width: 1
        color: "#eee"
        width: 820
        height: 500
    }

    Image {
        id: display
        property int cnt: 0
        anchors.centerIn: parent
        cache: false
        source: "image://display/" + cnt

        MouseArea {
            anchors.fill: parent
            onClicked: display.cnt++
        }
    }
}