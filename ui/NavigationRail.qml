import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    width: 230
    color: Theme.bgSurface
    border.color: Theme.borderSubtle
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.space12
        spacing: Theme.space8

        // Search Input (Fluent 2 search box)
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 34
            radius: Theme.radiusSm
            color: Theme.bgInput
            border.color: searchField.activeFocus ? Theme.borderFocus : Theme.borderDefault
            border.width: searchField.activeFocus ? 2 : 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.space8
                anchors.rightMargin: Theme.space8
                spacing: Theme.space8

                Image {
                    width: 14
                    height: 14
                    source: Theme.isDarkMode ? "icons/search_white.svg" : "icons/search.svg"
                    sourceSize.width: 28
                    sourceSize.height: 28
                    opacity: 0.7
                }

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: "Search cars, models..."
                    placeholderTextColor: Theme.textMuted
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                    background: null
                    selectByMouse: true
                    text: appController.searchText

                    onTextChanged: {
                        appController.searchText = text
                    }
                }

                Rectangle {
                    width: 16
                    height: 16
                    radius: Theme.radiusFull
                    color: Theme.bgHover
                    visible: searchField.text.length > 0

                    Text {
                        anchors.centerIn: parent
                        text: "✕"
                        font.pixelSize: 9
                        color: Theme.textSecondary
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            searchField.text = ""
                            appController.searchText = ""
                        }
                    }
                }
            }
        }

        // Section Title: VIEWS
        Text {
            Layout.topMargin: Theme.space8
            Layout.leftMargin: Theme.space4
            text: "FILTER BY STATUS"
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: 10
            font.weight: Font.DemiBold
            font.letterSpacing: 0.6
        }

        // Navigation Items List
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Theme.space4

            // Item 0: All Cars
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 36
                radius: Theme.radiusSm
                color: appController.filterMode === 0 ? Theme.bgActive : (item0Mouse.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space8
                    spacing: Theme.space8

                    // Active left indicator bar
                    Rectangle {
                        width: 3
                        height: 16
                        radius: 2
                        color: Theme.primary
                        visible: appController.filterMode === 0
                    }

                    Image {
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/car_white.svg" : "icons/car.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "All Cars"
                        color: appController.filterMode === 0 ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 0 ? Font.DemiBold : Font.Normal
                    }

                    // Count Badge
                    Rectangle {
                        implicitWidth: countText0.implicitWidth + 12
                        implicitHeight: 20
                        radius: Theme.radiusPill
                        color: Theme.bgCard
                        border.color: Theme.borderSubtle
                        border.width: 1

                        Text {
                            id: countText0
                            anchors.centerIn: parent
                            text: carModel.totalCount
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }
                    }
                }

                MouseArea {
                    id: item0Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 0
                }
            }

            // Item 1: Auto-Detected Matches
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 36
                radius: Theme.radiusSm
                color: appController.filterMode === 2 ? Theme.bgActive : (item1Mouse.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space8
                    spacing: Theme.space8

                    Rectangle {
                        width: 3
                        height: 16
                        radius: 2
                        color: Theme.primary
                        visible: appController.filterMode === 2
                    }

                    Image {
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                        opacity: 0.85
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Auto-Detected"
                        color: appController.filterMode === 2 ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 2 ? Font.DemiBold : Font.Normal
                    }

                    Rectangle {
                        implicitWidth: countText1.implicitWidth + 12
                        implicitHeight: 20
                        radius: Theme.radiusPill
                        color: Theme.bgCard
                        border.color: Theme.borderDefault
                        border.width: 1

                        Text {
                            id: countText1
                            anchors.centerIn: parent
                            text: carModel.detectedCount
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }
                    }
                }

                MouseArea {
                    id: item1Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 2
                }
            }

            // Item 2: Missing / Unknown
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 36
                radius: Theme.radiusSm
                color: appController.filterMode === 1 ? Theme.bgActive : (item2Mouse.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space8
                    spacing: Theme.space8

                    Rectangle {
                        width: 3
                        height: 16
                        radius: 2
                        color: Theme.primary
                        visible: appController.filterMode === 1
                    }

                    Image {
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                        opacity: 0.85
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Missing Brand"
                        color: appController.filterMode === 1 ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 1 ? Font.DemiBold : Font.Normal
                    }

                    Rectangle {
                        implicitWidth: countText2.implicitWidth + 12
                        implicitHeight: 20
                        radius: Theme.radiusPill
                        color: Theme.bgCard
                        border.color: Theme.borderDefault
                        border.width: 1

                        Text {
                            id: countText2
                            anchors.centerIn: parent
                            text: carModel.missingCount
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }
                    }
                }

                MouseArea {
                    id: item2Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 1
                }
            }

            // Item 3: Pending Changes
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 36
                radius: Theme.radiusSm
                color: appController.filterMode === 3 ? Theme.bgActive : (item3Mouse.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space8
                    spacing: Theme.space8

                    Rectangle {
                        width: 3
                        height: 16
                        radius: 2
                        color: Theme.primary
                        visible: appController.filterMode === 3
                    }

                    Image {
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/edit_white.svg" : "icons/edit.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                        opacity: 0.85
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Pending Changes"
                        color: appController.filterMode === 3 ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 3 ? Font.DemiBold : Font.Normal
                    }

                    Rectangle {
                        implicitWidth: countText3.implicitWidth + 12
                        implicitHeight: 20
                        radius: Theme.radiusPill
                        color: Theme.bgCard
                        border.color: Theme.borderDefault
                        border.width: 1

                        Text {
                            id: countText3
                            anchors.centerIn: parent
                            text: carModel.pendingCount
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }
                    }
                }

                MouseArea {
                    id: item3Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 3
                }
            }

            // Item 4: Verified Cars
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 36
                radius: Theme.radiusSm
                color: appController.filterMode === 4 ? Theme.bgActive : (item4Mouse.containsMouse ? Theme.bgHover : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space8
                    anchors.rightMargin: Theme.space8
                    spacing: Theme.space8

                    Rectangle {
                        width: 3
                        height: 16
                        radius: 2
                        color: Theme.primary
                        visible: appController.filterMode === 4
                    }

                    Image {
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/check_white.svg" : "icons/check.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                        opacity: 0.85
                    }

                    Text {
                        Layout.fillWidth: true
                        text: "Verified"
                        color: appController.filterMode === 4 ? Theme.textPrimary : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: appController.filterMode === 4 ? Font.DemiBold : Font.Normal
                    }

                    Rectangle {
                        implicitWidth: countText4.implicitWidth + 12
                        implicitHeight: 20
                        radius: Theme.radiusPill
                        color: Theme.bgCard
                        border.color: Theme.borderDefault
                        border.width: 1

                        Text {
                            id: countText4
                            anchors.centerIn: parent
                            text: carModel.savedCount
                            font.family: Theme.fontFamilyMono
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }
                    }
                }

                MouseArea {
                    id: item4Mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: appController.filterMode = 4
                }
            }
        }

        Item { Layout.fillHeight: true }

        // Batch Actions Card in Rail (Monochrome)
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: batchLayout.implicitHeight + Theme.space16
            radius: Theme.radiusMd
            color: Theme.bgCard
            border.color: Theme.borderDefault
            border.width: 1
            visible: carModel.pendingCount > 0

            ColumnLayout {
                id: batchLayout
                anchors.fill: parent
                anchors.margins: Theme.space8
                spacing: Theme.space8

                Text {
                    text: "PENDING MODS"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                // Batch Save Button (Monochrome High Contrast)
                Button {
                    Layout.fillWidth: true
                    implicitHeight: 30
                    padding: 0
                    visible: carModel.pendingCount > 0
                    text: "Save All Changes (" + carModel.pendingCount + ")"
                    background: Rectangle {
                        radius: Theme.radiusSm
                        color: parent.down ? Theme.primaryPressed : (parent.hovered ? Theme.primaryHover : Theme.primary)
                        border.color: Theme.borderDefault
                        border.width: 1
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Theme.primaryText
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: appController.saveAllPendingChanges()
                }
            }
        }
    }
}
