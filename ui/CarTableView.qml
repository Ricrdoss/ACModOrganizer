import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: Theme.bgApp

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Sticky Table Header
        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: Theme.bgSurface
            border.color: Theme.borderSubtle
            border.width: 1
            z: 10

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.space12
                anchors.rightMargin: Theme.space12
                spacing: Theme.space8

                Text {
                    Layout.preferredWidth: 100
                    text: "STATUS"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.preferredWidth: 50
                    text: "PREVIEW"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.fillWidth: true
                    text: "CAR NAME"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.preferredWidth: 160
                    text: "FOLDER"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.preferredWidth: 140
                    text: "BRAND"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.preferredWidth: 120
                    text: "COUNTRY"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.preferredWidth: 80
                    text: "ACTION"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignRight
                }
            }
        }

        // Table Rows List
        ListView {
            id: tableView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: carModel
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            spacing: 1

            ScrollBar.vertical: ScrollBar {
                active: true
                policy: ScrollBar.AsNeeded
            }

            delegate: Rectangle {
                id: rowRect
                width: tableView.width
                height: 42
                color: rowMouse.containsMouse ? Theme.bgHover : (index % 2 === 0 ? Theme.bgCard : Theme.bgApp)
                border.color: Theme.borderSubtle
                border.width: 1

                MouseArea {
                    id: rowMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.ArrowCursor
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: Theme.space12
                    anchors.rightMargin: Theme.space12
                    spacing: Theme.space8

                    // Status Pill (Monochrome pill)
                    Rectangle {
                        Layout.preferredWidth: 100
                        Layout.preferredHeight: 22
                        radius: Theme.radiusPill
                        color: Theme.bgSurface
                        border.color: Theme.borderDefault
                        border.width: 1

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: Theme.space4

                            Image {
                                width: 10
                                height: 10
                                source: {
                                    if (model.isPendingSave) return Theme.isDarkMode ? "icons/edit_white.svg" : "icons/edit.svg";
                                    if (model.hasSuggestion) return Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg";
                                    if (model.isBrandMissing || model.editedBrand === "Brand Not Found") return Theme.isDarkMode ? "icons/alert_white.svg" : "icons/alert.svg";
                                    return Theme.isDarkMode ? "icons/check_white.svg" : "icons/check.svg";
                                }
                                sourceSize.width: 20
                                sourceSize.height: 20
                                opacity: 0.8
                            }

                            Text {
                                text: model.statusString
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                font.weight: Font.DemiBold
                                color: Theme.textPrimary
                            }
                        }
                    }

                    // Thumbnail Preview (36x20)
                    Rectangle {
                        Layout.preferredWidth: 46
                        Layout.preferredHeight: 26
                        radius: Theme.radiusXs
                        color: Theme.bgApp
                        clip: true

                        Image {
                            anchors.fill: parent
                            source: model.previewUrl ? (model.previewUrl + "?t=" + appController.previewReloadToken) : ""
                            fillMode: Image.PreserveAspectCrop
                            asynchronous: true
                            cache: false
                            sourceSize.width: 92
                            sourceSize.height: 52
                        }
                    }

                    // Car Name
                    Text {
                        Layout.fillWidth: true
                        text: model.name || model.folderName
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                    }

                    // Folder Name
                    Text {
                        Layout.preferredWidth: 160
                        text: model.folderName
                        color: Theme.textMuted
                        font.family: Theme.fontFamilyMono
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }

                    // Inline Brand Box
                    Rectangle {
                        Layout.preferredWidth: 140
                        Layout.preferredHeight: 28
                        radius: Theme.radiusSm
                        color: Theme.bgInput
                        border.color: tableBrandInput.activeFocus ? Theme.borderFocus : Theme.borderDefault
                        border.width: tableBrandInput.activeFocus ? 2 : 1
                        clip: true

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Theme.space6
                            anchors.rightMargin: Theme.space6
                            spacing: Theme.space4

                            Image {
                                Layout.preferredWidth: 16
                                Layout.preferredHeight: 16
                                Layout.alignment: Qt.AlignVCenter
                                source: (model.badgeUrl && model.badgeUrl != "") ? model.badgeUrl : (appController.getBadgeForBrand(tableBrandInput.text) || "")
                                sourceSize.width: 32
                                sourceSize.height: 32
                                fillMode: Image.PreserveAspectFit
                                visible: source != "" && source != undefined
                                asynchronous: true
                            }

                            TextInput {
                                id: tableBrandInput
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter
                                text: model.editedBrand !== "" ? model.editedBrand : model.brand
                                color: text === "Brand Not Found" ? Theme.dangerText : Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                verticalAlignment: Text.AlignVCenter
                                selectByMouse: true
                                onEditingFinished: {
                                    if (text !== model.brand) {
                                        carModel.setEditedBrand(index, text)
                                        var autoCountry = appController.getCountryForBrand(text)
                                        if (autoCountry && autoCountry !== "") {
                                            tableCountryInput.text = autoCountry
                                            carModel.setEditedCountry(index, autoCountry)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Inline Country Box
                    Rectangle {
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 28
                        radius: Theme.radiusSm
                        color: Theme.bgInput
                        border.color: tableCountryInput.activeFocus ? Theme.borderFocus : Theme.borderDefault
                        border.width: tableCountryInput.activeFocus ? 2 : 1

                        TextInput {
                            id: tableCountryInput
                            anchors.fill: parent
                            anchors.leftMargin: Theme.space6
                            anchors.rightMargin: Theme.space6
                            text: model.editedCountry !== "" ? model.editedCountry : model.country
                            color: text === "Unknown" ? Theme.dangerText : Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            verticalAlignment: Text.AlignVCenter
                            selectByMouse: true
                            onEditingFinished: {
                                if (text !== model.country) {
                                    carModel.setEditedCountry(index, text)
                                }
                            }
                        }
                    }

                    // Row Action Button (Monochrome Save / Apply button)
                    Item {
                        Layout.preferredWidth: 80
                        Layout.preferredHeight: 28

                        // Save Pending Button
                        Button {
                            anchors.fill: parent
                            padding: 0
                            visible: model.isPendingSave
                            background: Rectangle {
                                radius: Theme.radiusSm
                                color: parent.down ? Theme.primaryPressed : (parent.hovered ? Theme.primaryHover : Theme.primary)
                                border.color: Theme.borderDefault
                                border.width: 1
                            }
                            contentItem: Text {
                                text: "Save"
                                color: Theme.primaryText
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                font.weight: Font.DemiBold
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: carModel.saveCar(index)
                        }

                        // Apply Suggestion Button
                        Button {
                            anchors.fill: parent
                            padding: 0
                            visible: !model.isPendingSave && model.hasSuggestion
                            background: Rectangle {
                                radius: Theme.radiusSm
                                color: parent.hovered ? Theme.bgCardHover : Theme.bgSurface
                                border.color: Theme.borderDefault
                                border.width: 1
                            }
                            contentItem: RowLayout {
                                anchors.centerIn: parent
                                spacing: Theme.space4
                                Image {
                                    width: 10
                                    height: 10
                                    source: Theme.isDarkMode ? "icons/zap_white.svg" : "icons/zap.svg"
                                    sourceSize.width: 20
                                    sourceSize.height: 20
                                }
                                Text {
                                    text: "Apply"
                                    color: Theme.textPrimary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                    font.weight: Font.Medium
                                }
                            }
                            onClicked: carModel.applySuggestion(index)
                        }
                    }
                }
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
                        tableView.positionViewAtIndex(0, ListView.Beginning);
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
                                tableView.positionViewAtIndex(0, ListView.Beginning);
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
                        tableView.positionViewAtIndex(0, ListView.Beginning);
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
