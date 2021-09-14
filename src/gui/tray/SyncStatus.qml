import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import Style 1.0

import com.nextcloud.desktopclient 1.0 as NC

Item {
    id: item

    property var model: NC.SyncStatusModel {}

    implicitHeight: layout.height

    RowLayout {
        id: layout

        Layout.alignment: Qt.AlignLeft

        width: item.width
        spacing: 0

        Image {
            id: syncIcon

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.topMargin: 16
            Layout.bottomMargin: 16
            Layout.leftMargin: 16

            source: model.syncIcon
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

                    text: model.syncStatusString
                    visible: !model.syncing
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: Style.topLinePixelSize
                    font.bold: true
                }

                Text {
                    id: syncStatusDetailText

                    text: model.syncStatusDetailString
                    visible: !model.syncing && model.syncStatusDetailString !== ""
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

                    text: model.syncStatusString
                    visible: model.syncing
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: Style.topLinePixelSize
                    font.bold: true
                }

                ProgressBar {
                    id: syncProgressBar

                    Layout.rightMargin: 16
                    Layout.fillWidth: true

                    value: model.syncProgress
                    visible: model.syncing
                }

                Text {
                    id: syncProgressDetailText

                    Layout.bottomMargin: 8
                    Layout.fillWidth: true

                    text: model.syncString
                    visible: model.syncing
                    color: "#808080"
                    font.pixelSize: Style.subLinePixelSize
                }
            }
        }
    }
}
