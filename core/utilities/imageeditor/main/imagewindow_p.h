/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam image editor GUI
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_WINDOW_PRIVATE_H
#define DIGIKAM_IMAGE_WINDOW_PRIVATE_H

// C++ includes

#include <cstdio>
#include <vector>
#include <algorithm>

// Qt includes

#include <QKeySequence>
#include <QCloseEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QPixmap>
#include <QProgressBar>
#include <QSplitter>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QApplication>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kwindowsystem.h>

// Local includes

#include "versionmanager.h"
#include "dlayoutbox.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumfiltermodel.h"
#include "applicationsettings.h"
#include "canvas.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "componentsinfodlg.h"
#include "coredbaccess.h"
#include "coredbwatch.h"
#include "coredbchangesets.h"
#include "ddragobjects.h"
#include "deletedialog.h"
#include "dimg.h"
#include "editorcore.h"
#include "dimagehistory.h"
#include "digikamapp.h"
#include "dio.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "iccsettingscontainer.h"
#include "itemattributeswatch.h"
#include "itemfiltermodel.h"
#include "itemdragdrop.h"
#include "itemdescedittab.h"
#include "iteminfo.h"
#include "itemgps.h"
#include "itemlistmodel.h"
#include "itempropertiessidebardb.h"
#include "itempropertiesversionstab.h"
#include "itemscanner.h"
#include "itemthumbnailbar.h"
#include "iofilesettings.h"
#include "dnotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "metadatahub.h"
#include "metaenginesettings.h"
#include "metadataedit.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"
#include "presentationmngr.h"
#include "ratingwidget.h"
#include "savingcontext.h"
#include "scancontroller.h"
#include "setup.h"
#include "slideshow.h"
#include "statusprogressbar.h"
#include "syncjob.h"
#include "tagsactionmngr.h"
#include "tagscache.h"
#include "tagspopupmenu.h"
#include "tagregion.h"
#include "thememanager.h"
#include "thumbbardock.h"
#include "thumbnailloadthread.h"
#include "undostate.h"
#include "dexpanderbox.h"
#include "dbinfoiface.h"
#include "calwizard.h"
#include "expoblendingmanager.h"
#include "mailwizard.h"
#include "advprintwizard.h"
#include "dmediaserverdlg.h"
#include "facetagseditor.h"
#include "timeadjustdialog.h"
#include "wsstarter.h"

#ifdef HAVE_MARBLE
#   include "geolocationedit.h"
#endif

#ifdef HAVE_HTMLGALLERY
#   include "htmlwizard.h"
#endif

#ifdef HAVE_PANORAMA
#   include "panomanager.h"
#endif

#ifdef HAVE_MEDIAPLAYER
#   include "vidslidewizard.h"
#endif

namespace Digikam
{

class Q_DECL_HIDDEN DatabaseVersionManager : public VersionManager
{
public:

    virtual QString toplevelDirectory(const QString& path)
    {
        CollectionLocation loc = CollectionManager::instance()->locationForPath(path);

        if (!loc.isNull())
        {
            return loc.albumRootPath();
        }

        return QLatin1String("/");
    }
};

// -----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ImageWindow::Private
{

public:

    Private()
      : configShowThumbbarEntry(QLatin1String("Show Thumbbar")),
        configHorizontalThumbbarEntry(QLatin1String("HorizontalThumbbar")),
        viewContainer(0),
        toMainWindowAction(0),
        fileDeletePermanentlyAction(0),
        fileDeletePermanentlyDirectlyAction(0),
        fileTrashDirectlyAction(0),
        imageInfoModel(0),
        imageFilterModel(0),
        dragDropHandler(0),
        thumbBar(0),
        thumbBarDock(0),
        rightSideBar(0)
    {
    }

    QModelIndex currentIndex() const
    {
        return imageFilterModel->indexForItemInfo(currentItemInfo);
    }

    QModelIndex currentSourceIndex() const
    {
        return imageInfoModel->indexForItemInfo(currentItemInfo);
    }

    bool currentIsValid() const
    {
        return !currentItemInfo.isNull();
    }

    QUrl currentUrl() const
    {
        return currentItemInfo.fileUrl();
    }

    QModelIndex nextIndex() const
    {
        return imageFilterModel->index(currentIndex().row() + 1, 0);
    }

    QModelIndex previousIndex() const
    {
        return imageFilterModel->index(currentIndex().row() - 1, 0);
    }

    QModelIndex firstIndex() const
    {
        return imageFilterModel->index(0, 0);
    }

    QModelIndex lastIndex() const
    {
        return imageFilterModel->index(imageFilterModel->rowCount() - 1, 0);
    }

    ItemInfo imageInfo(const QModelIndex& index) const
    {
        return imageFilterModel->imageInfo(index);
    }

    void setThumbBarToCurrent()
    {
        QModelIndex index = imageFilterModel->indexForItemInfo(currentItemInfo);

        if (index.isValid())
        {
            thumbBar->setCurrentIndex(index);
        }
        else
        {
            thumbBar->setCurrentWhenAvailable(currentItemInfo.id());
        }
    }

    void ensureModelContains(const ItemInfo& info)
    {
        if (!imageInfoModel->hasImage(info))
        {
            imageInfoModel->addItemInfoSynchronously(info);
            imageFilterModel->sort(imageFilterModel->sortColumn());
        }
    }

public:

    const QString                configShowThumbbarEntry;
    const QString                configHorizontalThumbbarEntry;

    KMainWindow*                 viewContainer;

    QAction*                     toMainWindowAction;

    // Delete actions
    QAction*                     fileDeletePermanentlyAction;
    QAction*                     fileDeletePermanentlyDirectlyAction;
    QAction*                     fileTrashDirectlyAction;

    ItemInfo                    currentItemInfo;
    ItemListModel*              imageInfoModel;
    ItemFilterModel*            imageFilterModel;
    ItemDragDropHandler*        dragDropHandler;

    ItemThumbnailBar*           thumbBar;
    ThumbBarDock*                thumbBarDock;

    ItemPropertiesSideBarDB*    rightSideBar;

    DatabaseVersionManager       versionManager;

    QMultiMap<QString, QVariant> newFaceTags;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_WINDOW_PRIVATE_H
