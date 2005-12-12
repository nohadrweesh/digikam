/* ============================================================
 * File  : adjustcurves.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright 2004-2005 by Gilles Caulier
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

// C++ includes.

#include <cmath>
#include <cstring>

// Qt includes.

#include <qlayout.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qhbuttongroup.h> 
#include <qpixmap.h>
#include <qcheckbox.h>

// KDE includes.

#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kselect.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Local includes.

#include "version.h"
#include "adjustcurves.h"

namespace DigikamAdjustCurvesImagesPlugin
{

AdjustCurveDialog::AdjustCurveDialog(QWidget* parent)
                 : ImageTabDialog(parent, i18n("Adjust Color Curves"), "adjustcurves", 
                                  true, true, false)
{
    Digikam::ImageIface iface(0, 0);
    uchar *data     = iface.getOriginalImage();
    int w           = iface.originalWidth();
    int h           = iface.originalHeight();
    bool sixteenBit = iface.originalSixteenBit();
    bool hasAlpha   = iface.originalHasAlpha();
    m_originalImage = Digikam::DImg(w, h, sixteenBit, hasAlpha ,data);
    delete [] data;

    m_histoSegments = m_originalImage.sixteenBit() ? 65535 : 255;
    m_curves = new Digikam::ImageCurves(m_originalImage.sixteenBit());

    // About data and help button.

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Adjust Color Curves"),
                                       digikamimageplugins_version,
                                       I18N_NOOP("An image-histogram-curves adjustment plugin for digiKam."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");

    setAboutData(about);
    
    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(plainPage());
    QGridLayout* grid = new QGridLayout( gboxSettings, 7, 5, marginHint(), spacingHint());

    QLabel *label1 = new QLabel(i18n("Channel:"), gboxSettings);
    label1->setAlignment ( Qt::AlignRight | Qt::AlignVCenter );
    m_channelCB = new QComboBox( false, gboxSettings );
    m_channelCB->insertItem( i18n("Luminosity") );
    m_channelCB->insertItem( i18n("Red") );
    m_channelCB->insertItem( i18n("Green") );
    m_channelCB->insertItem( i18n("Blue") );
    m_channelCB->insertItem( i18n("Alpha") );
    m_channelCB->setCurrentText( i18n("Luminosity") );
    QWhatsThis::add( m_channelCB, i18n("<p>Select here the histogram channel to display:<p>"
                                       "<b>Luminosity</b>: display the image's luminosity values.<p>"
                                       "<b>Red</b>: display the red image-channel values.<p>"
                                       "<b>Green</b>: display the green image-channel values.<p>"
                                       "<b>Blue</b>: display the blue image-channel values.<p>"
                                       "<b>Alpha</b>: display the alpha image-channel values. "
                                       "This channel corresponds to the transparency value and "
                                       "is supported by some image formats, such as PNG or GIF."));

    m_scaleBG = new QHButtonGroup(gboxSettings);
    m_scaleBG->setExclusive(true);
    m_scaleBG->setFrameShape(QFrame::NoFrame);
    m_scaleBG->setInsideMargin( 0 );
    QWhatsThis::add( m_scaleBG, i18n("<p>Select here the histogram scale.<p>"
                                     "If the image's maximal counts are small, you can use the linear scale.<p>"
                                     "Logarithmic scale can be used when the maximal counts are big; "
                                     "if it is used, all values (small and large) will be visible on the graph."));
    
    QPushButton *linHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( linHistoButton, i18n( "<p>Linear" ) );
    m_scaleBG->insert(linHistoButton, Digikam::CurvesWidget::LinScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-lin", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("histogram-lin", "histogram-lin.png");
    linHistoButton->setPixmap( QPixmap( directory + "histogram-lin.png" ) );
    linHistoButton->setToggleButton(true);
    
    QPushButton *logHistoButton = new QPushButton( m_scaleBG );
    QToolTip::add( logHistoButton, i18n( "<p>Logarithmic" ) );
    m_scaleBG->insert(logHistoButton, Digikam::CurvesWidget::LogScaleHistogram);
    KGlobal::dirs()->addResourceType("histogram-log", KGlobal::dirs()->kde_default("data") + "digikam/data");
    directory = KGlobal::dirs()->findResourceDir("histogram-log", "histogram-log.png");
    logHistoButton->setPixmap( QPixmap( directory + "histogram-log.png" ) );
    logHistoButton->setToggleButton(true);       

    QHBoxLayout* l1 = new QHBoxLayout();
    l1->addWidget(m_scaleBG);
    l1->addStretch(10);
    
    grid->addMultiCellWidget(label1, 0, 0, 1, 1);
    grid->addMultiCellWidget(m_channelCB, 0, 0, 2, 2);
    grid->addMultiCellLayout(l1, 0, 0, 4, 5);
    
    QLabel *label5 = new QLabel(i18n("Type:"), gboxSettings);
    label5->setAlignment ( Qt::AlignRight | Qt::AlignVCenter);
    m_typeCB = new QComboBox( false, gboxSettings );
    m_typeCB->insertItem( i18n("Smooth") );
    m_typeCB->insertItem( i18n("Free") );
    m_typeCB->setCurrentText( i18n("Smooth") );
    QWhatsThis::add( m_typeCB, i18n("<p>Select here the curve type to draw for the current channel.<p>"
                                    "<b>Smooth</b>: this mode constrains the curve type to a smooth line with tension.<p>"
                                    "<b>Free</b>: with this mode, you can draw your curve free-hand with the mouse."));

    m_labelPos = new QLabel(gboxSettings);
                                     
    grid->addMultiCellWidget(label5, 1, 1, 1, 1);
    grid->addMultiCellWidget(m_typeCB, 1, 1, 2, 2);
    grid->addMultiCellWidget(m_labelPos, 2, 2, 1, 5);
    
    m_vGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Vertical, 10, gboxSettings );
    m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
    grid->addMultiCellWidget(m_vGradient, 3, 3, 0, 0);

    m_curvesWidget = new Digikam::CurvesWidget(256, 256, m_originalImage.bits(), m_originalImage.width(),
                                               m_originalImage.height(), m_originalImage.sixteenBit(),
                                               m_curves, gboxSettings);
    QWhatsThis::add( m_curvesWidget, i18n("<p>This is the curve drawing of the selected image "
                                          "histogram channel"));
    grid->addMultiCellWidget(m_curvesWidget, 3, 3, 1, 5);
    
    m_hGradient = new Digikam::ColorGradientWidget( Digikam::ColorGradientWidget::Horizontal, 10, gboxSettings );
    m_hGradient->setColors( QColor( "black" ), QColor( "white" ) );
    grid->addMultiCellWidget(m_hGradient, 4, 4, 1, 5);
    
    // -------------------------------------------------------------
    
    m_pickerColorButtonGroup = new QHButtonGroup(gboxSettings);
    m_pickBlack = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickBlack, BlackTonal);
    KGlobal::dirs()->addResourceType("color-picker-black", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-black", "color-picker-black.png");
    m_pickBlack->setPixmap( QPixmap( directory + "color-picker-black.png" ) );
    m_pickBlack->setToggleButton(true);
    QToolTip::add( m_pickBlack, i18n( "All channels shadow tone color picker" ) );
    QWhatsThis::add( m_pickBlack, i18n("<p>With this button, you can pick the color from original image used to set <b>Shadow Tone</b> "
                                       "smooth curves point on Red, Green, Blue, and Luminosity channels."));
    m_pickGray  = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickGray, GrayTonal);
    KGlobal::dirs()->addResourceType("color-picker-gray", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-gray", "color-picker-gray.png");
    m_pickGray->setPixmap( QPixmap( directory + "color-picker-gray.png" ) );
    m_pickGray->setToggleButton(true);
    QToolTip::add( m_pickGray, i18n( "All channels middle tone color picker" ) );
    QWhatsThis::add( m_pickGray, i18n("<p>With this button, you can pick the color from original image used to set <b>Middle Tone</b> "
                                      "smooth curves point on Red, Green, Blue, and Luminosity channels."));
    m_pickWhite = new QPushButton(m_pickerColorButtonGroup);
    m_pickerColorButtonGroup->insert(m_pickWhite, WhiteTonal);
    KGlobal::dirs()->addResourceType("color-picker-white", KGlobal::dirs()->kde_default("data") +
                                     "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("color-picker-white", "color-picker-white.png");
    m_pickWhite->setPixmap( QPixmap( directory + "color-picker-white.png" ) );
    m_pickWhite->setToggleButton(true);
    QToolTip::add( m_pickWhite, i18n( "All channels highlight tone color picker" ) );
    QWhatsThis::add( m_pickWhite, i18n("<p>With this button, you can pick the color from original image used to set <b>Highlight Tone</b> "
                                       "smooth curves point on Red, Green, Blue, and Luminosity channels."));
    m_pickerColorButtonGroup->setExclusive(true);
    m_pickerColorButtonGroup->setFrameShape(QFrame::NoFrame);

    m_resetButton = new QPushButton(i18n("&Reset"), gboxSettings);
    QWhatsThis::add( m_resetButton, i18n("<p>Reset curves' values from the current selected channel."));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->addWidget(m_pickerColorButtonGroup);
    l3->addWidget(m_resetButton);
    l3->addStretch(10);
    
    grid->addMultiCellLayout(l3, 5, 5, 1, 5);
    
    // -------------------------------------------------------------
            
    m_overExposureIndicatorBox = new QCheckBox(i18n("Over exposure indicator"), gboxSettings);
    QWhatsThis::add( m_overExposureIndicatorBox, i18n("<p>If you enable this option, over-exposed pixels "
                                                      "from the target image preview will be over-colored. "
                                                      "This will not have an effect on the final rendering."));
    grid->addMultiCellWidget(m_overExposureIndicatorBox, 6, 6, 1, 5);
    grid->setRowStretch(7, 10);
    
    setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    m_previewOriginalWidget = previewOriginalWidget();
    QWhatsThis::add( m_previewOriginalWidget, i18n("<p>You can see here the original image. You can pick "
                                                   "a color on the image using the color "
                                                   "picker tools to select shadow, middle, and highlight "
                                                   "tones to adjust the curves' points in the Red, "
                                                   "Green, Blue, and Luminosity Channels."));
    m_previewTargetWidget   = previewTargetWidget();
    QWhatsThis::add( m_previewTargetWidget, i18n("<p>You can see here the image's "
                                                 "curve-adjustments preview."));

    // -------------------------------------------------------------
    
    // Reset all parameters to the default values.
    QTimer::singleShot(0, this, SLOT(slotDefault()));

    // -------------------------------------------------------------
    
    connect(m_curvesWidget, SIGNAL(signalMouseMoved(int, int)),
            this, SLOT(slotPositionChanged(int, int)));
                        
    connect(m_curvesWidget, SIGNAL(signalCurvesChanged()),
            this, SLOT(slotEffect()));
    
    connect(m_previewOriginalWidget, SIGNAL(spotPositionChanged( const Digikam::DColor &, bool, const QPoint & )),
            this, SLOT(slotSpotColorChanged( const Digikam::DColor &, bool )));
            
    connect(m_overExposureIndicatorBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));              

    connect(m_previewTargetWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));                                                            
            
    // -------------------------------------------------------------
    // ComboBox slots.

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged(int)));

    connect(m_scaleBG, SIGNAL(released(int)),
            this, SLOT(slotScaleChanged(int)));
            
    connect(m_typeCB, SIGNAL(activated(int)),
            this, SLOT(slotCurveTypeChanged(int)));
    
    // -------------------------------------------------------------
    // Bouttons slots.

    connect(m_resetButton, SIGNAL(clicked()),
            this, SLOT(slotResetCurrentChannel()));
}

AdjustCurveDialog::~AdjustCurveDialog()
{
    delete m_curvesWidget;
    delete m_curves;
}

void AdjustCurveDialog::slotSpotColorChanged(const Digikam::DColor &color, bool release)
{
    Digikam::DColor sc = color;

    if ( m_pickBlack->isOn() )
    {
       // Black tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 1, 
                               QPoint::QPoint(QMAX(QMAX(sc.red(), sc.green()), sc.blue()), 42*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 1, QPoint::QPoint(sc.red(), 42*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 1, QPoint::QPoint(sc.green(), 42*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 1, QPoint::QPoint(sc.blue(), 42*m_histoSegments/256));
       m_pickBlack->setOn(!release);
    }
    else if ( m_pickGray->isOn() )
    {
       // Gray tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 8, 
                               QPoint::QPoint(QMAX(QMAX(sc.red(), sc.green()), sc.blue()), 128*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 8, QPoint::QPoint(sc.red(), 128*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 8, QPoint::QPoint(sc.green(), 128*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 8, QPoint::QPoint(sc.blue(), 128*m_histoSegments/256));
       m_pickGray->setOn(!release);
    }
    else if ( m_pickWhite->isOn() )
    {
       // White tonal curves point.
       m_curves->setCurvePoint(Digikam::ImageHistogram::ValueChannel, 15,
                               QPoint::QPoint(QMAX(QMAX(sc.red(), sc.green()), sc.blue()), 213*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::RedChannel, 15, QPoint::QPoint(sc.red(), 213*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::GreenChannel, 15, QPoint::QPoint(sc.green(), 213*m_histoSegments/256));
       m_curves->setCurvePoint(Digikam::ImageHistogram::BlueChannel, 15, QPoint::QPoint(sc.blue(), 213*m_histoSegments/256));
       m_pickWhite->setOn(!release);
    }
    else
       m_curvesWidget->setCurveGuide(color);

    // Calculate Red, green, blue curves.
    
    for (int i = Digikam::ImageHistogram::ValueChannel ; i <= Digikam::ImageHistogram::BlueChannel ; i++)
       m_curves->curvesCalculateCurve(i);
    
    m_curvesWidget->repaint(false);
       
    slotEffect();  
}

void AdjustCurveDialog::slotResetCurrentChannel()
{
    m_curves->curvesChannelReset(m_channelCB->currentItem());

    m_curvesWidget->reset();
    slotEffect();
}

void AdjustCurveDialog::slotPositionChanged(int x, int y)
{
    if ( x == -1 && y == -1)
       m_labelPos->clear();
    else
       m_labelPos->setText(i18n("Position: x:%1  y:%2").arg(x).arg(y));
}

void AdjustCurveDialog::slotEffect()
{
    uchar* desData;

    Digikam::ImageIface* iface = m_previewTargetWidget->imageIface();
    uchar *orgData             = iface->getPreviewImage();
    int w                      = iface->previewWidth();
    int h                      = iface->previewHeight();

    // Create the new empty destination image data space.
    if (m_histoSegments == 65535)
       desData = new uchar[w*h*8];
    else
       desData = new uchar[w*h*4];

    // Calculate the LUT to apply on the image.
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel, m_overExposureIndicatorBox->isChecked());

    // Apply the lut to the image.
    m_curves->curvesLutProcess(orgData, desData, w, h);

    iface->putPreviewImage(desData);
    m_previewTargetWidget->updatePreview();

    delete [] orgData;
    delete [] desData;
}

void AdjustCurveDialog::slotOk()
{
    uchar* desData;

    kapp->setOverrideCursor( KCursor::waitCursor() );
    Digikam::ImageIface* iface = m_previewTargetWidget->imageIface();
    uchar *orgData             = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    
    // Create the new empty destination image data space.
    if (m_histoSegments == 65535)
       desData = new uchar[w*h*8];
    else
       desData = new uchar[w*h*4];

    // Calculate the LUT to apply on the image.
    m_curves->curvesLutSetup(Digikam::ImageHistogram::AlphaChannel);

    // Apply the lut to the image.
    m_curves->curvesLutProcess(orgData, desData, w, h);

    iface->putOriginalImage(i18n("Adjust Curve"), desData);
    kapp->restoreOverrideCursor();

    delete [] orgData;
    delete [] desData;
    accept();
}

void AdjustCurveDialog::slotChannelChanged(int channel)
{
    switch(channel)
    {
       case LuminosityChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::ValueHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;
       
       case RedChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::RedChannelHistogram;
          m_vGradient->setColors( QColor( "red" ), QColor( "black" ) );
          break;

       case GreenChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::GreenChannelHistogram;
          m_vGradient->setColors( QColor( "green" ), QColor( "black" ) );
          break;

       case BlueChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::BlueChannelHistogram;
          m_vGradient->setColors( QColor( "blue" ), QColor( "black" ) );
          break;

       case AlphaChannel:
          m_curvesWidget->m_channelType = Digikam::CurvesWidget::AlphaChannelHistogram;
          m_vGradient->setColors( QColor( "white" ), QColor( "black" ) );
          break;
    }

    m_typeCB->setCurrentItem(m_curves->getCurveType(channel));  
                
    m_curvesWidget->repaint(false);
}

void AdjustCurveDialog::slotScaleChanged(int scale)
{
    m_curvesWidget->m_scaleType = scale;
    m_curvesWidget->repaint(false);
}

void AdjustCurveDialog::slotCurveTypeChanged(int type)
{
    switch(type)
    {
       case SmoothDrawing:          
       {
          m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_SMOOTH);
          m_pickerColorButtonGroup->setEnabled(true);
          break;
       }
       
       case FreeDrawing:          
       {
          m_curves->setCurveType(m_curvesWidget->m_channelType, Digikam::ImageCurves::CURVE_FREE);
          m_pickerColorButtonGroup->setEnabled(false);
          break;
       }
    }
    
    m_curvesWidget->curveTypeChanged();
}

// Reset all settings.
void AdjustCurveDialog::slotDefault()
{
    for (int channel = 0 ; channel < 5 ; channel++)
       m_curves->curvesChannelReset(channel);

    m_curvesWidget->reset();
    slotEffect();
}

// Load all settings.
void AdjustCurveDialog::slotUser3()
{
    KURL loadCurvesFile;

    loadCurvesFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Select Gimp Curves File to Load")) );
    if( loadCurvesFile.isEmpty() )
       return;

    if ( m_curves->loadCurvesFromGimpCurvesFile( loadCurvesFile ) == false )
    {
       KMessageBox::error(this, i18n("Cannot load from the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged(m_channelCB->currentItem());
    slotEffect();
}

// Save all settings.
void AdjustCurveDialog::slotUser2()
{
    KURL saveCurvesFile;

    saveCurvesFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                             QString( "*" ), this,
                                             QString( i18n("Gimp Curves File to Save")) );
    if( saveCurvesFile.isEmpty() )
       return;

    if ( m_curves->saveCurvesToGimpCurvesFile( saveCurvesFile ) == false )
    {
       KMessageBox::error(this, i18n("Cannot save to the Gimp curves text file."));
       return;
    }

    // Refresh the current curves config.
    slotChannelChanged(m_channelCB->currentItem());
}

}  // NameSpace DigikamAdjustCurvesImagesPlugin

#include "adjustcurves.moc"

