import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Style 1.0

import com.nextcloud.desktopclient 1.0 as NC

RowLayout {
    id: layout

    property alias model: syncStatusModel

    spacing: 0

    NC.SyncStatusModel {
        id: syncStatusModel
    }

    Image {
        id: syncIcon

        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        Layout.topMargin: 16
        Layout.bottomMargin: 16
        Layout.leftMargin: 16

        source: syncStatusModel.syncIcon
        sourceSize.width: 32
        sourceSize.height: 32
    }

    Item {
        id: containerItem

        Layout.alignment: Qt.AlignVCenter
        Layout.leftMargin: 10
        Layout.fillWidth: true
        Layout.fillHeight: true

        ColumnLayout {
            id: columnLayout

            anchors.verticalCenter: containerItem.verticalCenter

            spacing: 0

            Text {
                id: syncStatusText

                text: syncStatusModel.syncStatusString
                visible: !syncStatusModel.syncing
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Style.topLinePixelSize
                font.bold: true
            }

            Text {
                id: syncStatusDetailText

                text: syncStatusModel.syncStatusDetailString
                visible: !syncStatusModel.syncing && syncStatusModel.syncStatusDetailString !== ""
                color: "#808080"
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Style.subLinePixelSize
            }
        }

        ColumnLayout {
            id: syncProgressLayout

            anchors.fill: parent

            Text {
                id: syncProgressText
                
                Layout.topMargin: 8
                Layout.fillWidth: true

                text: syncStatusModel.syncStatusString
                visible: syncStatusModel.syncing
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: Style.topLinePixelSize
                font.bold: true
            }

            ProgressBar {
                id: syncProgressBar

                Layout.rightMargin: 16
                Layout.fillWidth: true

                value: syncStatusModel.syncProgress
                visible: syncStatusModel.syncing
            }

            Text {
                id: syncProgressDetailText

                Layout.bottomMargin: 8
                Layout.fillWidth: true

                text: syncStatusModel.syncString
                visible: syncStatusModel.syncing
                color: "#808080"
                font.pixelSize: Style.subLinePixelSize
            }
        }
    }
}
