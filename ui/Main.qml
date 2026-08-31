import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 1380
    height: 880
    minimumWidth: 1060
    minimumHeight: 680
    visible: true
    title: "Assetto Corsa Mod Organizer"
    color: Theme.bgApp

    FolderDialog {
        id: folderDialog
        title: "Select Assetto Corsa 'content/cars' Directory"
        currentFolder: appController.carsDir ? ("file:///" + appController.carsDir.replace(/\\/g, "/")) : ""
        onAccepted: {
            var selectedPath = selectedFolder.toString()
            if (selectedPath.startsWith("file:///")) {
                selectedPath = selectedPath.substring(8)
            }
            appController.carsDir = selectedPath
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 1. Top Windows 11 CommandBar / TitleBar
        TopBar {
            id: topBar
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            onBrowseRequested: folderDialog.open()
            onScanRequested: appController.startScan()
            onSaveAllRequested: appController.saveAllPending()
        }

        // Horizontal Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Theme.borderSubtle
        }

        // 2. Central Workspace (Navigation Rail on Left + Content View on Right)
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // WinUI 3 Left Navigation Rail
            NavigationRail {
                id: navRail
                Layout.preferredWidth: 230
                Layout.fillHeight: true
            }

            // Vertical Divider
            Rectangle {
                width: 1
                Layout.fillHeight: true
                color: Theme.borderSubtle
            }

            // Main Content Area
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // View 1: Empty State (No cars loaded)
                Item {
                    anchors.centerIn: parent
                    visible: carModel.totalCount === 0 && !appController.isScanning
                    width: 440
                    height: 240

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: Theme.space16

                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            width: 52
                            height: 52
                            radius: Theme.radiusMd
                            color: Theme.bgCard
                            border.color: Theme.borderDefault
                            border.width: 1

                            Image {
                                anchors.centerIn: parent
                                width: 26
                                height: 26
                                source: Theme.isDarkMode ? "icons/car_white.svg" : "icons/car.svg"
                                sourceSize.width: 52
                                sourceSize.height: 52
                            }
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "No Cars Loaded"
                            font.family: Theme.fontFamilyDisplay
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: Theme.textPrimary
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 360
                            text: "Verify the directory path in the top bar and click 'Scan Cars' to inspect your Assetto Corsa mods."
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            color: Theme.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                            lineHeight: 1.4
                            wrapMode: Text.WordWrap
                        }

                        Button {
                            Layout.alignment: Qt.AlignHCenter
                            visible: appController.carsDir !== ""
                            background: Rectangle {
                                implicitWidth: 160
                                implicitHeight: 32
                                color: parent.down ? Theme.primaryPressed : (parent.hovered ? Theme.primaryHover : Theme.primary)
                                radius: Theme.radiusSm
                            }
                            contentItem: RowLayout {
                                spacing: Theme.space6
                                anchors.centerIn: parent
                                Image {
                                    width: 13
                                    height: 13
                                    source: Theme.isDarkMode ? "icons/refresh_dark.svg" : "icons/refresh_white.svg"
                                    sourceSize.width: 26
                                    sourceSize.height: 26
                                }
                                Text {
                                    text: "Scan Directory"
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 12
                                    font.weight: Font.DemiBold
                                    color: Theme.primaryText
                                }
                            }
                            onClicked: appController.startScan()
                        }
                    }
                }

                // View 2: Scanning Progress Overlay
                Item {
                    anchors.centerIn: parent
                    visible: appController.isScanning
                    width: 360
                    height: 160

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: Theme.space12

                        BusyIndicator {
                            Layout.alignment: Qt.AlignHCenter
                            running: appController.isScanning
                            width: 36
                            height: 36
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Scanning Mod Directory..."
                            font.family: Theme.fontFamilyDisplay
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: Theme.textPrimary
                        }

                        ProgressBar {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 280
                            from: 0
                            to: 100
                            value: appController.scanProgressPercent
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: 320
                            text: appController.scanStatusText
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            color: Theme.textMuted
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideMiddle
                        }

                        Button {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Cancel"
                            onClicked: appController.cancelScan()
                            background: Rectangle {
                                implicitWidth: 72
                                implicitHeight: 26
                                color: parent.hovered ? Theme.bgHover : Theme.bgCard
                                radius: Theme.radiusSm
                                border.color: Theme.borderDefault
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                font.weight: Font.Medium
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                    }
                }

                // View 3: High-Density Table / List View
                CarTableView {
                    anchors.fill: parent
                    visible: Theme.isTableView && carModel.totalCount > 0 && !appController.isScanning
                }

                // View 4: Dense Responsive Card Grid View (PAGINATED - 30 cars per page)
                Item {
                    anchors.fill: parent
                    visible: !Theme.isTableView && carModel.totalCount > 0 && !appController.isScanning

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0

                        // Card Grid
                        GridView {
                            id: carGrid
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            leftMargin: Theme.space12
                            rightMargin: Theme.space12
                            topMargin: Theme.space12
                            bottomMargin: Theme.space12

                            readonly property int minCardWidth: 320
                            readonly property int computedCols: Math.max(1, Math.floor(width / minCardWidth))
                            cellWidth: (width - Theme.space12 * 2) / computedCols
                            cellHeight: 410
                            cacheBuffer: 410 * 2

                            model: carModel

                            delegate: Item {
                                width: carGrid.cellWidth
                                height: carGrid.cellHeight

                                CarCard {
                                    anchors.fill: parent
                                    anchors.margins: Theme.space6
                                }
                            }

                            ScrollBar.vertical: ScrollBar {
                                active: true
                                policy: ScrollBar.AsNeeded
                            }
                        }

                        // Pagination Footer Bar
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 44
                            color: Theme.bgSurface
                            border.color: Theme.borderSubtle
                            border.width: 1

                            RowLayout {
                                anchors.centerIn: parent
                                spacing: Theme.space4

                                // Previous button
                                Button {
                                    padding: 0
                                    enabled: carModel.currentPage > 1
                                    background: Rectangle {
                                        implicitWidth: 32
                                        implicitHeight: 32
                                        radius: Theme.radiusSm
                                        color: parent.hovered && parent.enabled ? Theme.bgHover : "transparent"
                                    }
                                    contentItem: Text {
                                        text: "‹"
                                        color: parent.enabled ? Theme.textPrimary : Theme.textDisabled
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 16
                                        font.weight: Font.Bold
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        carModel.prevPage();
                                        carGrid.positionViewAtIndex(0, GridView.Beginning);
                                    }
                                }

                                // Page number buttons
                                Repeater {
                                    model: {
                                        var pages = [];
                                        var tp = carModel.totalPages;
                                        var cp = carModel.currentPage;
                                        if (tp <= 7) {
                                            for (var i = 1; i <= tp; i++) pages.push(i);
                                        } else {
                                            pages.push(1);
                                            if (cp > 3) pages.push(-1);
                                            var start = Math.max(2, cp - 1);
                                            var end = Math.min(tp - 1, cp + 1);
                                            if (cp <= 3) { start = 2; end = 4; }
                                            if (cp >= tp - 2) { start = tp - 3; end = tp - 1; }
                                            for (var j = start; j <= end; j++) pages.push(j);
                                            if (cp < tp - 2) pages.push(-2);
                                            pages.push(tp);
                                        }
                                        return pages;
                                    }

                                    delegate: Item {
                                        width: 32
                                        height: 32
                                        readonly property int pageNum: modelData

                                        Text {
                                            anchors.centerIn: parent
                                            visible: pageNum < 0
                                            text: "…"
                                            color: Theme.textMuted
                                            font.family: Theme.fontFamily
                                            font.pixelSize: 12
                                        }

                                        Button {
                                            anchors.fill: parent
                                            visible: pageNum > 0
                                            padding: 0
                                            background: Rectangle {
                                                radius: Theme.radiusSm
                                                color: {
                                                    if (pageNum === carModel.currentPage) return Theme.primary;
                                                    if (parent.hovered) return Theme.bgHover;
                                                    return "transparent";
                                                }
                                            }
                                            contentItem: Text {
                                                text: pageNum > 0 ? String(pageNum) : ""
                                                color: pageNum === carModel.currentPage ? Theme.primaryText : Theme.textSecondary
                                                font.family: Theme.fontFamily
                                                font.pixelSize: 12
                                                font.weight: pageNum === carModel.currentPage ? Font.Bold : Font.Medium
                                                horizontalAlignment: Text.AlignHCenter
                                                verticalAlignment: Text.AlignVCenter
                                            }
                                            onClicked: {
                                                carModel.goToPage(pageNum);
                                                carGrid.positionViewAtIndex(0, GridView.Beginning);
                                            }
                                        }
                                    }
                                }

                                // Next button
                                Button {
                                    padding: 0
                                    enabled: carModel.currentPage < carModel.totalPages
                                    background: Rectangle {
                                        implicitWidth: 32
                                        implicitHeight: 32
                                        radius: Theme.radiusSm
                                        color: parent.hovered && parent.enabled ? Theme.bgHover : "transparent"
                                    }
                                    contentItem: Text {
                                        text: "›"
                                        color: parent.enabled ? Theme.textPrimary : Theme.textDisabled
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 16
                                        font.weight: Font.Bold
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    onClicked: {
                                        carModel.nextPage();
                                        carGrid.positionViewAtIndex(0, GridView.Beginning);
                                    }
                                }

                                Text {
                                    leftPadding: Theme.space12
                                    text: {
                                        if (carModel.filteredCount === 0) return "0 cars";
                                        var start = (carModel.currentPage - 1) * carModel.pageSize + 1;
                                        var end = Math.min(carModel.currentPage * carModel.pageSize, carModel.filteredCount);
                                        return "Showing " + start + "–" + end + " of " + carModel.filteredCount;
                                    }
                                    color: Theme.textMuted
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }

                // View 5: Empty Search Filter Result
                Item {
                    anchors.centerIn: parent
                    visible: carModel.totalCount > 0 && carModel.filteredCount === 0 && !appController.isScanning
                    width: 300
                    height: 120

                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: Theme.space8

                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            width: 28
                            height: 28
                            source: Theme.isDarkMode ? "icons/search_white.svg" : "icons/search.svg"
                            sourceSize.width: 56
                            sourceSize.height: 56
                            opacity: 0.6
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "No Matching Cars"
                            font.family: Theme.fontFamilyDisplay
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: Theme.textPrimary
                        }

                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "Try adjusting your search query or filter."
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            color: Theme.textMuted
                        }
                    }
                }
            }
        }
    }

    // Windows 11 Style Toast Notification (Dynamic width based on message)
    Rectangle {
        id: toast
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: Theme.space20
        implicitWidth: toastContent.implicitWidth + Theme.space24
        width: Math.min(parent.width - 40, implicitWidth)
        height: 36
        radius: Theme.radiusPill
        visible: false
        opacity: 0
        z: 9999

        property string toastType: "info"

        color: Theme.bgSurface
        border.color: Theme.borderDefault
        border.width: 1

        RowLayout {
            id: toastContent
            anchors.centerIn: parent
            spacing: Theme.space8

            Image {
                width: 14
                height: 14
                source: {
                    if (toast.toastType === "success") return Theme.isDarkMode ? "icons/check_white.svg" : "icons/check.svg";
                    if (toast.toastType === "error") return Theme.isDarkMode ? "icons/close_white.svg" : "icons/close.svg";
                    if (toast.toastType === "warning") return Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg";
                    return Theme.isDarkMode ? "icons/shield_white.svg" : "icons/shield.svg";
                }
                sourceSize.width: 28
                sourceSize.height: 28
            }

            Text {
                id: toastText
                text: ""
                font.family: Theme.fontFamily
                font.pixelSize: 12
                font.weight: Font.DemiBold
                color: Theme.textPrimary
            }
        }

        SequentialAnimation {
            id: toastAnim
            NumberAnimation { target: toast; property: "opacity"; to: 1.0; duration: 150 }
            PauseAnimation { duration: 2800 }
            NumberAnimation { target: toast; property: "opacity"; to: 0.0; duration: 200 }
            ScriptAction { script: toast.visible = false }
        }

        function show(type, message) {
            toastType = type;
            toastText.text = message;
            toast.visible = true;
            toastAnim.restart();
        }
    }

    UpdateNotificationDialog {
        id: updateNotificationDialog
    }

    // Bottom-right Version & Update Status Watermark
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: Theme.space12
        height: 24
        radius: Theme.radiusSm
        color: versionMouseArea.containsMouse ? Theme.bgCardHover : Theme.bgSurface
        border.color: Theme.borderDefault
        border.width: 1
        implicitWidth: versionRow.implicitWidth + 16

        RowLayout {
            id: versionRow
            anchors.centerIn: parent
            spacing: Theme.space6

            Rectangle {
                width: 6
                height: 6
                radius: 3
                color: (appController.updateManager && appController.updateManager.updateAvailable) ? "#2563eb" : "#22c55e"
            }

            Text {
                text: appController.appVersion + " • " + ((appController.updateManager && appController.updateManager.updateAvailable) ? "Update Available" : "Check for Updates")
                color: (appController.updateManager && appController.updateManager.updateAvailable) ? Theme.textPrimary : Theme.textMuted
                font.family: Theme.fontFamilyMono
                font.pixelSize: 10
                font.weight: Font.Medium
            }
        }

        MouseArea {
            id: versionMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                if (appController.updateManager && appController.updateManager.updateAvailable) {
                    updateNotificationDialog.open()
                } else {
                    appController.checkForUpdates(false)
                }
            }
        }
    }

    Connections {
        target: appController
        function onShowToast(type, title, message) {
            toast.show(type, title + ": " + message);
        }
    }

    Connections {
        target: appController.updateManager
        function onUpdateCheckCompleted(hasUpdate, version) {
            if (hasUpdate) {
                updateNotificationDialog.open()
            }
        }
    }
}
