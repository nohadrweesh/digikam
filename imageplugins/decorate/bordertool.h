/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-20
 * Description : a digiKam image plugin to add a border
 *               around an image.
 *
 * Copyright 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef BORDERTOOL_H
#define BORDERTOOL_H

// Qt includes

#include <QString>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamDecorateImagePlugin
{

class BorderTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    BorderTool(QObject* parent);
    ~BorderTool();

private Q_SLOTS:

    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();

private:

    class BorderToolPriv;
    BorderToolPriv* const d;
};

}  // namespace DigikamDecorateImagePlugin

#endif /* BORDERTOOL_H */
