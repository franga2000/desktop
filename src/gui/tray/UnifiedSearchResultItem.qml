import QtQml 2.12
import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2
import Style 1.0
import com.nextcloud.desktopclient 1.0

MouseArea {
    id: unifiedSearchResultMouseArea
    hoverEnabled: true
    anchors.fill: unifiedSearchResultItem
    
    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        color: (parent.containsMouse ? Style.lightHover : "transparent")
    }

    RowLayout {
        id: unifiedSearchResultItem

        RowLayout {
            id: unifiedSearchResultItemDetails

            visible: !model.isFetchMoreTrigger && !model.isCategorySeparator

            width: !model.isFetchMoreTrigger && !model.isCategorySeparator ? unifiedSearchResultMouseArea.width : 0
            height: !model.isFetchMoreTrigger && !model.isCategorySeparator ? Style.trayWindowHeaderHeight : 0

            Accessible.role: Accessible.ListItem
            Accessible.name: resultTitle
            Accessible.onPressAction: unifiedSearchResultMouseArea.clicked()

            Column {
                id: unifiedSearchResultLeftColumn

                Layout.leftMargin: 8
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                spacing: 4
                Image {
                    id: unifiedSearchResultThumbnail
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    Layout.preferredWidth: shareButton.icon.width
                    Layout.preferredHeight: shareButton.icon.height
                    verticalAlignment: Qt.AlignCenter
                    cache: true
                    source: model.thumbnailUrl ? model.thumbnailUrl : "qrc:///client/theme/share.svg"
                    sourceSize.height: Style.trayWindowHeaderHeight
                    sourceSize.width: Style.trayWindowHeaderHeight
                }
            }

            Column {
                id: unifiedSearchResultRightColumn
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 8
                Layout.bottomMargin: 8
                spacing: 4
                ColumnLayout {
                    Rectangle {
                        Layout.preferredHeight: unifiedSearchResultRightColumn.height / 2
                        Text {
                            id: unifiedSearchResultTitleText
                            text: model.resultTitle
                            anchors.top: parent.top
                            visible: parent.visible
                            width: parent.width
                            font.pixelSize: Style.topLinePixelSize
                            color: "red"
                        }
                    }

                    Rectangle {
                        Layout.preferredHeight: unifiedSearchResultRightColumn.height / 2 - unifiedSearchResultTextSubline.height

                        Text {
                            id: unifiedSearchResultTextSubline
                            text: model.subline
                            anchors.bottom: parent.bottom
                            visible: parent.visible
                            width: parent.width
                            font.pixelSize: Style.topLinePixelSize
                            color: "grey"
                        }
                    }


                }


            }

        }

        RowLayout {
            id: unifiedSearchResultItemFetchMore
            visible: model.isFetchMoreTrigger

            width: model.isFetchMoreTrigger ? unifiedSearchResultMouseArea.width : 0
            height: model.isFetchMoreTrigger ? Style.trayWindowHeaderHeight : 0
            spacing: 2

            Accessible.role: Accessible.ListItem
            Accessible.name: "Fetch more items"
            Accessible.onPressAction: unifiedSearchResultMouseArea.clicked()

            Column {
                id: unifiedSearchResultItemFetchMoreColumn
                visible: model.isFetchMoreTrigger
                Layout.leftMargin: 8
                Layout.topMargin: 4
                Layout.bottomMargin: 4
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4
                Layout.alignment: Qt.AlignLeft

                Text {
                    id: unifiedSearchResultItemFetchMoreText
                    text: "Fetch more..."
                    visible: parent.visible
                    width: parent.width
                    font.pixelSize: Style.topLinePixelSize
                    color: "grey"
                }
            }
        }

        RowLayout {
            id: unifiedSearchResultItemCategorySeparator
            visible: model.isCategorySeparator

            width: model.isCategorySeparator ? unifiedSearchResultMouseArea.width : 0
            height: model.isCategorySeparator ? Style.trayWindowHeaderHeight : 0
            spacing: 2

            Accessible.role: Accessible.ListItem
            Accessible.name: "Category separator."
            Accessible.onPressAction: unifiedSearchResultMouseArea.clicked()

            Column {
                id: unifiedSearchResultItemCategorySeparatorColumn
                visible: model.isCategorySeparator
                Layout.leftMargin: 8
                Layout.topMargin: 4
                Layout.bottomMargin: 4
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4
                Layout.alignment: Qt.AlignLeft

                Text {
                    id: unifiedSearchResultItemCategorySeparatorText
                    text: model.categoryName
                    visible: parent.visible
                    width: parent.width
                    font.pixelSize: Style.topLinePixelSize
                    color: "blue"
                }
            }
        }
    }
}
