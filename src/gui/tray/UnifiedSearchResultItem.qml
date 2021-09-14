import QtQml 2.12
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2
import Style 1.0
import com.nextcloud.desktopclient 1.0

MouseArea {
    id: unifiedSearchResultMouseArea
    hoverEnabled: true
    
    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: (parent.containsMouse ? Style.lightHover : "transparent")
    }
        
    RowLayout {
        id: unifiedSearchResultItem
        
        width: unifiedSearchResultMouseArea.width
        height: Style.trayWindowHeaderHeight
        spacing: 0
        
        Accessible.role: Accessible.ListItem
        Accessible.name: title
        Accessible.onPressAction: unifiedSearchResultMouseArea.clicked()   
        
        Column {
            id: unifiedSearchResultTextColumn
            Layout.leftMargin: 8
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 4
            Layout.alignment: Qt.AlignLeft
            
            Text {
                id: activityTextTitle
                text: name + " " + subject
                width: parent.width
                elide: Text.ElideRight
                font.pixelSize: Style.topLinePixelSize
                color: "black"
            }
        }
    }
}
