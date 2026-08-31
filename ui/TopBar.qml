import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    height: 48
    color: Theme.bgSurface
    border.color: Theme.borderSubtle
    border.width: 1

    signal browseRequested()
    signal scanRequested()
    signal saveAllRequested()

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.space16
        anchors.rightMargin: Theme.space16
        spacing: Theme.space12

        // App Brand & Logo
        RowLayout {
            spacing: Theme.space8
            Layout.alignment: Qt.AlignVCenter

            Rectangle {
                width: 28
                height: 28
                radius: Theme.radiusSm
                color: Theme.primarySurface
                border.color: Theme.primaryBorder
                border.width: 1

                Image {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    source: Theme.isDarkMode ? "icons/car_white.svg" : "icons/car.svg"
                    sourceSize.width: 28
                    sourceSize.height: 28
                }
            }

            Text {
                text: "Assetto Corsa"
                font.family: Theme.fontFamilyDisplay
                font.pixelSize: 13
                font.weight: Font.Bold
                color: Theme.textPrimary
            }

            Text {
                text: "Mod Organizer"
                font.family: Theme.fontFamily
                font.pixelSize: 12
                color: Theme.textSecondary
            }
        }

        // Vertical Divider
        Rectangle {
            width: 1
            height: 20
            color: Theme.borderSubtle
        }

        // Directory Path Picker Input (Fluent Input Bar)
        Rectangle {
            Layout.fillWidth: true
            Layout.maximumWidth: 520
            height: 32
            radius: Theme.radiusSm
            color: Theme.bgInput
            border.color: pathInput.activeFocus ? Theme.borderFocus : (pathArea.containsMouse ? Theme.borderHover : Theme.borderDefault)
            border.width: pathInput.activeFocus ? 2 : 1

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.space8
                anchors.rightMargin: Theme.space4
                spacing: Theme.space8

                Image {
                    width: 14
                    height: 14
                    source: Theme.isDarkMode ? "icons/folder_white.svg" : "icons/folder.svg"
                    sourceSize.width: 28
                    sourceSize.height: 28
                    opacity: 0.8
                }

                TextInput {
                    id: pathInput
                    Layout.fillWidth: true
                    text: appController.carsDir
                    color: Theme.textPrimary
                    font.family: Theme.fontFamilyMono
                    font.pixelSize: 11
                    selectByMouse: true
                    clip: true
                    verticalAlignment: Text.AlignVCenter
                    onTextEdited: appController.carsDir = text

                    Text {
                        anchors.fill: parent
                        visible: !pathInput.text && !pathInput.activeFocus
                        text: "Path to content/cars..."
                        color: Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                // Browse Button
                Button {
                    id: browseBtn
                    implicitWidth: 68
                    implicitHeight: 24
                    padding: 0
                    background: Rectangle {
                        radius: Theme.radiusSm
                        color: browseBtn.down ? Theme.bgActive : (browseBtn.hovered ? Theme.bgHover : Theme.bgCard)
                        border.color: Theme.borderDefault
                        border.width: 1
                    }
                    contentItem: Text {
                        text: "Browse"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: root.browseRequested()
                }
            }

            MouseArea {
                id: pathArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.IBeamCursor
                acceptedButtons: Qt.NoButton
            }
        }

        // Primary Scan Button
        Button {
            id: scanBtn
            implicitHeight: 32
            implicitWidth: 116
            padding: 0
            enabled: !appController.isScanning && appController.carsDir !== ""
            background: Rectangle {
                radius: Theme.radiusSm
                color: {
                    if (!scanBtn.enabled) return Theme.bgHover;
                    if (scanBtn.down) return Theme.primaryPressed;
                    return scanBtn.hovered ? Theme.primaryHover : Theme.primary;
                }
                border.color: Theme.borderDefault
                border.width: 1
            }
            contentItem: RowLayout {
                spacing: Theme.space6
                Item { Layout.fillWidth: true }
                Image {
                    Layout.alignment: Qt.AlignVCenter
                    width: 13
                    height: 13
                    source: Theme.isDarkMode ? "icons/refresh_dark.svg" : "icons/refresh_white.svg"
                    sourceSize.width: 26
                    sourceSize.height: 26
                }
                Text {
                    Layout.alignment: Qt.AlignVCenter
                    verticalAlignment: Text.AlignVCenter
                    text: appController.isScanning ? "Scanning..." : "Scan Cars"
                    color: Theme.primaryText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                Item { Layout.fillWidth: true }
            }
            onClicked: root.scanRequested()
        }



        Item { Layout.fillWidth: true }

        // View Mode Switcher (Grid vs Table)
        Rectangle {
            implicitHeight: 32
            implicitWidth: 72
            radius: Theme.radiusSm
            color: Theme.bgInput
            border.color: Theme.borderDefault
            border.width: 1

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // Grid Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.radiusSm
                    color: !Theme.isTableView ? Theme.bgActive : "transparent"

                    Image {
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/grid_white.svg" : "icons/grid.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                        opacity: !Theme.isTableView ? 1.0 : 0.5
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Theme.isTableView = false
                    }
                }

                // Table Button
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: Theme.radiusSm
                    color: Theme.isTableView ? Theme.bgActive : "transparent"

                    Image {
                        anchors.centerIn: parent
                        width: 14
                        height: 14
                        source: Theme.isDarkMode ? "icons/table_white.svg" : "icons/table.svg"
                        sourceSize.width: 28
                        sourceSize.height: 28
                        opacity: Theme.isTableView ? 1.0 : 0.5
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Theme.isTableView = true
                    }
                }
            }
        }

        // Theme Toggle Button (Light/Dark mode)
        Button {
            id: themeToggleBtn
            implicitWidth: 32
            implicitHeight: 32
            padding: 0
            background: Rectangle {
                radius: Theme.radiusSm
                color: themeToggleBtn.down ? Theme.bgActive : (themeToggleBtn.hovered ? Theme.bgHover : Theme.bgInput)
                border.color: Theme.borderDefault
                border.width: 1
            }
            contentItem: Item {
                Image {
                    anchors.centerIn: parent
                    width: 14
                    height: 14
                    source: Theme.isDarkMode ? "icons/sun_white.svg" : "icons/moon.svg"
                    sourceSize.width: 28
                    sourceSize.height: 28
                }
            }
            onClicked: Theme.toggleTheme()
        }

        // Save All Changes Button (Monochrome contrast button)
        Button {
            id: saveAllBtn
            implicitHeight: 32
            padding: 0
            visible: carModel.pendingCount > 0
            background: Rectangle {
                implicitWidth: saveLayout.implicitWidth + 24
                radius: Theme.radiusSm
                color: saveAllBtn.down ? Theme.primaryPressed : (saveAllBtn.hovered ? Theme.primaryHover : Theme.primary)
                border.color: Theme.borderDefault
                border.width: 1
            }
            contentItem: RowLayout {
                id: saveLayout
                spacing: Theme.space6
                Item { Layout.fillWidth: true }
                Image {
                    Layout.alignment: Qt.AlignVCenter
                    width: 13
                    height: 13
                    source: Theme.isDarkMode ? "icons/save_dark.svg" : "icons/save_white.svg"
                    sourceSize.width: 26
                    sourceSize.height: 26
                }
                Text {
                    Layout.alignment: Qt.AlignVCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Save All (" + carModel.pendingCount + ")"
                    color: Theme.primaryText
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                }
                Item { Layout.fillWidth: true }
            }
            onClicked: root.saveAllRequested()
        }
    }
}
