#pragma once

#include <QAbstractListModel>
#include <QString>
#include <vector>
#include <memory>
#include "CarItem.hpp"

namespace acbo {

enum class FilterMode {
    All = 0,
    Unassigned = 1,
    AutoDetected = 2,
    PendingChanges = 3,
    Verified = 4
};

class CarListModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int totalCount READ totalCount NOTIFY countsChanged)
    Q_PROPERTY(int missingCount READ missingCount NOTIFY countsChanged)
    Q_PROPERTY(int detectedCount READ detectedCount NOTIFY countsChanged)
    Q_PROPERTY(int pendingCount READ pendingCount NOTIFY countsChanged)
    Q_PROPERTY(int savedCount READ savedCount NOTIFY countsChanged)
    Q_PROPERTY(int filteredCount READ totalFilteredCars NOTIFY filterChanged)
    Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY pageChanged)
    Q_PROPERTY(int totalPages READ totalPages NOTIFY pageChanged)
    Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize NOTIFY pageSizeChanged)

public:
    enum CarRoles {
        NameRole = Qt::UserRole + 1,
        FolderNameRole,
        FolderPathRole,
        JsonPathRole,
        PreviewUrlRole,
        BrandRole,
        CountryRole,
        AuthorRole,
        YearRole,
        TagsRole,
        SuggestedBrandRole,
        SuggestedCountryRole,
        DetectionConfidenceRole,
        DetectionReasonRole,
        HasSuggestionRole,
        EditedBrandRole,
        EditedCountryRole,
        IsPendingSaveRole,
        IsSavedRole,
        StatusStringRole,
        StatusTypeRole,
        IsValidJsonRole,
        ParseErrorRole,
        OriginalIndexRole,
        IsBrandMissingRole,
        IsCountryMissingRole,
        BadgeUrlRole,
        CountryFlagRole
    };

    explicit CarListModel(QObject* parent = nullptr);
    ~CarListModel() override = default;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    void setCars(std::vector<CarItem> cars);
    void clear();

    // Filtering & Searching
    void setSearchText(const QString& text);
    void setSearchFilter(const QString& text) { setSearchText(text); }
    void setFilterMode(FilterMode mode);
    void invalidateFilter() { recomputeFilteredIndices(); }

    // Pagination
    [[nodiscard]] int totalFilteredCars() const { return static_cast<int>(m_filteredIndices.size()); }
    [[nodiscard]] int currentPage() const { return m_currentPage; }
    [[nodiscard]] int totalPages() const;
    [[nodiscard]] int pageSize() const { return m_pageSize; }
    void setCurrentPage(int page);
    void setPageSize(int size);

    [[nodiscard]] int totalCount() const;
    [[nodiscard]] int missingCount() const;
    [[nodiscard]] int detectedCount() const;
    [[nodiscard]] int pendingCount() const;
    [[nodiscard]] int savedCount() const;
    [[nodiscard]] int verifiedCount() const { return static_cast<int>(m_allCars.size()) - m_missingCount - m_detectedCount; }
    [[nodiscard]] const std::vector<CarItem>& rawCars() const { return m_allCars; }

    // Interactive operations callable from QML
    Q_INVOKABLE void nextPage();
    Q_INVOKABLE void prevPage();
    Q_INVOKABLE void goToPage(int page);
    Q_INVOKABLE void applySuggestion(int filteredRow);
    Q_INVOKABLE void setEditedBrand(int filteredRow, const QString& newBrand);
    Q_INVOKABLE void setEditedCountry(int filteredRow, const QString& newCountry);
    Q_INVOKABLE bool saveCar(int filteredRow);
    Q_INVOKABLE void applyAllSuggestions();
    Q_INVOKABLE int saveAllPending();
    Q_INVOKABLE void openFolder(int filteredRow);
    Q_INVOKABLE void updatePreviewForCar(const QString& folderName, const QString& newPreviewPath);
    Q_INVOKABLE void refreshAllPreviews();

signals:
    void countsChanged();
    void filterChanged();
    void pageChanged();
    void pageSizeChanged();
    void saveError(const QString& folderName, const QString& error);
    void saveSuccess(const QString& folderName);

private:
    std::vector<CarItem> m_allCars;
    std::vector<size_t> m_filteredIndices;

    QString m_searchText;
    FilterMode m_filterMode{FilterMode::All};

    int m_currentPage{1};
    int m_pageSize{30};

    int m_missingCount{0};
    int m_detectedCount{0};
    int m_pendingCount{0};
    int m_savedCount{0};

    void recomputeFilteredIndices();
    void updateCounters();
    [[nodiscard]] bool passesFilter(const CarItem& car) const;
};

} // namespace acbo
