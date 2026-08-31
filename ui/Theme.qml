pragma Singleton
import QtQuick

QtObject {
    // ----------------------------------------------------
    // Theme Mode Management (Dark / Light)
    // ----------------------------------------------------
    property bool isDarkMode: true
    property bool isTableView: false

    function toggleTheme() {
        isDarkMode = !isDarkMode
    }

    function toggleViewMode() {
        isTableView = !isTableView
    }

    // ----------------------------------------------------
    // Typography Fonts (Windows 11 Native Segoe UI Variable)
    // ----------------------------------------------------
    readonly property string fontFamily: "Segoe UI Variable Text, Segoe UI, -apple-system, sans-serif"
    readonly property string fontFamilyDisplay: "Segoe UI Variable Display, Segoe UI, -apple-system, sans-serif"
    readonly property string fontFamilyMono: "Cascadia Code, Consolas, Courier New, monospace"

    // ----------------------------------------------------
    // Spacing Scale (Strict 4px / 8px Windows grid)
    // ----------------------------------------------------
    readonly property int space2: 2
    readonly property int space4: 4
    readonly property int space6: 6
    readonly property int space8: 8
    readonly property int space12: 12
    readonly property int space16: 16
    readonly property int space20: 20
    readonly property int space24: 24
    readonly property int space32: 32
    readonly property int space40: 40
    readonly property int space48: 48

    // ----------------------------------------------------
    // Border Radii (Fluent 2 Desktop geometry)
    // ----------------------------------------------------
    readonly property int radiusXs: 3
    readonly property int radiusSm: 4
    readonly property int radiusMd: 6
    readonly property int radiusLg: 8
    readonly property int radiusPill: 9999
    readonly property int radiusFull: 9999

    // ----------------------------------------------------
    // Monochrome Surface & Background Palette
    // ----------------------------------------------------
    readonly property color bgApp: isDarkMode ? "#121212" : "#f8f9fa"
    readonly property color bgSurface: isDarkMode ? "#181818" : "#ffffff"
    readonly property color bgCard: isDarkMode ? "#1d1d1f" : "#ffffff"
    readonly property color bgCardHover: isDarkMode ? "#242426" : "#f4f4f5"
    readonly property color bgInput: isDarkMode ? "#141416" : "#ffffff"
    readonly property color bgHover: isDarkMode ? "#27272a" : "#f4f4f5"
    readonly property color bgActive: isDarkMode ? "#323236" : "#e4e4e7"

    // ----------------------------------------------------
    // Monochrome 1px Desktop Borders
    // ----------------------------------------------------
    readonly property color borderSubtle: isDarkMode ? "#262628" : "#e4e4e7"
    readonly property color borderDefault: isDarkMode ? "#333336" : "#d4d4d8"
    readonly property color borderHover: isDarkMode ? "#52525b" : "#a1a1aa"
    readonly property color borderFocus: isDarkMode ? "#ededed" : "#18181b"

    // ----------------------------------------------------
    // Monochrome Typography (High Contrast WCAG AA)
    // ----------------------------------------------------
    readonly property color textPrimary: isDarkMode ? "#ededed" : "#111827"
    readonly property color textSecondary: isDarkMode ? "#a1a1aa" : "#4b5563"
    readonly property color textMuted: isDarkMode ? "#71717a" : "#9ca3af"
    readonly property color textDisabled: isDarkMode ? "#52525b" : "#d1d5db"

    // ----------------------------------------------------
    // Primary Monochrome Accent (Solid High-Contrast)
    // ----------------------------------------------------
    readonly property color primary: isDarkMode ? "#ededed" : "#18181b"
    readonly property color primaryHover: isDarkMode ? "#ffffff" : "#27272a"
    readonly property color primaryPressed: isDarkMode ? "#d4d4d8" : "#3f3f46"
    readonly property color primarySurface: isDarkMode ? "#27272a" : "#f4f4f5"
    readonly property color primaryBorder: isDarkMode ? "#3f3f46" : "#e4e4e7"
    readonly property color primaryText: isDarkMode ? "#09090b" : "#ffffff"

    // ----------------------------------------------------
    // Auto-Detected / Verified State (Refined Monochrome Slate)
    // ----------------------------------------------------
    readonly property color success: isDarkMode ? "#d4d4d8" : "#27272a"
    readonly property color successHover: isDarkMode ? "#e4e4e7" : "#3f3f46"
    readonly property color successPressed: isDarkMode ? "#a1a1aa" : "#52525b"
    readonly property color successSurface: isDarkMode ? "#222225" : "#f4f4f5"
    readonly property color successBorder: isDarkMode ? "#3a3a40" : "#e4e4e7"
    readonly property color successText: isDarkMode ? "#ededed" : "#18181b"

    // ----------------------------------------------------
    // Pending Changes State (Muted Zinc, NOT loud yellow)
    // ----------------------------------------------------
    readonly property color warning: isDarkMode ? "#ededed" : "#18181b"
    readonly property color warningHover: isDarkMode ? "#ffffff" : "#27272a"
    readonly property color warningPressed: isDarkMode ? "#d4d4d8" : "#3f3f46"
    readonly property color warningSurface: isDarkMode ? "#28282c" : "#f4f4f5"
    readonly property color warningBorder: isDarkMode ? "#44444a" : "#d4d4d8"
    readonly property color warningText: isDarkMode ? "#ededed" : "#18181b"

    // ----------------------------------------------------
    // Missing / Alert State (Subtle Neutral Zinc)
    // ----------------------------------------------------
    readonly property color danger: isDarkMode ? "#a1a1aa" : "#71717a"
    readonly property color dangerHover: isDarkMode ? "#d4d4d8" : "#52525b"
    readonly property color dangerPressed: isDarkMode ? "#71717a" : "#3f3f46"
    readonly property color dangerSurface: isDarkMode ? "#202022" : "#f4f4f5"
    readonly property color dangerBorder: isDarkMode ? "#38383c" : "#e4e4e7"
    readonly property color dangerText: isDarkMode ? "#a1a1aa" : "#71717a"
}
