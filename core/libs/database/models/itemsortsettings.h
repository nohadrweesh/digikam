/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : Sort settings for use with ItemFilterModel
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_SORT_SETTINGS_H
#define DIGIKAM_ITEM_SORT_SETTINGS_H

// Qt includes

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QCollator>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class ItemInfo;

namespace DatabaseFields
{
    class Set;
}

class DIGIKAM_DATABASE_EXPORT ItemSortSettings
{
public:

    explicit ItemSortSettings();

    bool operator==(const ItemSortSettings& other) const;

    /** Compares the categories of left and right.
     *  Return -1 if left is less than right, 0 if both fall in the same category,
     *  and 1 if left is greater than right.
     *  Adheres to set categorization mode and current category sort order.
     */
    int compareCategories(const ItemInfo& left, const ItemInfo& right) const;

    /** Returns true if left is less than right.
     *  Adheres to current sort role and sort order.
     */
    bool lessThan(const ItemInfo& left, const ItemInfo& right) const;

    /** Compares the ItemInfos left and right.
     *  Return -1 if left is less than right, 1 if left is greater than right,
     *  and 0 if left equals right comparing the current sort role's value.
     *  Adheres to set sort role and sort order.
     */
    int compare(const ItemInfo& left, const ItemInfo& right) const;

    /** Returns true if left QVariant is less than right.
     *  Adheres to current sort role and sort order.
     *  Use for extraValue, if necessary.
     */
    bool lessThan(const QVariant& left, const QVariant& right) const;

    enum SortOrder
    {
        AscendingOrder  = Qt::AscendingOrder,
        DescendingOrder = Qt::DescendingOrder,
        DefaultOrder /// sort order depends on the chosen sort role
    };

    /// --- Categories ---

    enum CategorizationMode
    {
        NoCategories, /// categorization switched off
        OneCategory, /// all items in one global category
        CategoryByAlbum,
        CategoryByFormat,
        CategoryByMonth
    };

    CategorizationMode      categorizationMode;
    SortOrder               categorizationSortOrder;

    void setCategorizationMode(CategorizationMode mode);
    void setCategorizationSortOrder(SortOrder order);

    /// Only Ascending or Descending, never DefaultOrder
    Qt::SortOrder           currentCategorizationSortOrder;
    Qt::CaseSensitivity     categorizationCaseSensitivity;

    bool isCategorized() const { return categorizationMode >= CategoryByAlbum; }

    /// --- Image Sorting ---

    enum SortRole
    {
        // Note: For legacy reasons, the order of the first five entries must remain unchanged
        SortByFileName,
        SortByFilePath,
        SortByCreationDate,
        SortByFileSize,
        SortByRating,
        SortByImageSize,            // pixel number
        SortByAspectRatio,          // width / height * 100000
        SortBySimilarity,
        SortByManualOrder,
        SortByModificationDate      // currently not used in the icon view
    };

    SortRole                sortRole;
    SortOrder               sortOrder;
    bool                    strTypeNatural;

    void setSortRole(SortRole role);
    void setSortOrder(SortOrder order);
    void setStringTypeNatural(bool natural);

    Qt::SortOrder           currentSortOrder;
    Qt::CaseSensitivity     sortCaseSensitivity;

    int compare(const ItemInfo& left, const ItemInfo& right, SortRole sortRole) const;

    // --- ---

    static Qt::SortOrder defaultSortOrderForCategorizationMode(CategorizationMode mode);
    static Qt::SortOrder defaultSortOrderForSortRole(SortRole role);

    /// --- Change notification ---

    /** Returns database fields a change in which would affect the current sorting.
     */
    DatabaseFields::Set watchFlags() const;

    /// --- Utilities ---

    /** Returns a < b if sortOrder is Ascending, or b < a if order is descending.
     */
    template <typename T>
    static inline bool lessThanByOrder(const T& a, const T& b, Qt::SortOrder sortOrder)
    {
        if (sortOrder == Qt::AscendingOrder)
        {
            return a < b;
        }

        return b < a;
    }

    /** Returns the usual compare result of -1, 0, or 1 for lessThan, equals and greaterThan.
     */
    template <typename T>
    static inline int compareValue(const T& a, const T& b)
    {
        if (a == b)
        {
            return 0;
        }

        if (a > b)
        {
            return 1;
        }

        return -1;
    }

    /** Takes a typical result from a compare method (0 is equal, -1 is less than, 1 is greater than)
     *  and applies the given sort order to it.
     */
    static inline int compareByOrder(int compareResult,  Qt::SortOrder sortOrder)
    {
        if (sortOrder == Qt::AscendingOrder)
        {
            return compareResult;
        }

        return - compareResult;
    }

    template <typename T>
    static inline int compareByOrder(const T& a, const T& b, Qt::SortOrder sortOrder)
    {
        return compareByOrder(compareValue(a, b), sortOrder);
    }

    /** Compares the two string by natural comparison and adheres to given sort order
     */
    static inline int naturalCompare(const QString& a, const QString& b, Qt::SortOrder sortOrder,
                                     Qt::CaseSensitivity caseSensitive = Qt::CaseSensitive,
                                     bool natural = true)
    {
        QCollator collator;
        collator.setNumericMode(natural);
        collator.setIgnorePunctuation(false);
        collator.setCaseSensitivity(caseSensitive);

        if (a.contains(QLatin1String("_v"), Qt::CaseInsensitive))
        {
            collator.setIgnorePunctuation(true);
        }

        if (sortOrder == Qt::AscendingOrder)
        {
            return collator.compare(a, b);
        }

        return - collator.compare(a, b);
    }
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_SORT_SETTINGS_H
