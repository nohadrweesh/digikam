/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Julien Narboux <julien at narboux dot fr>
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

#include "filmgrainfilter.h"

// C++ includes

#include <cmath>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "globals.h"

namespace Digikam
{

FilmGrainFilter::FilmGrainFilter(DImg* orgImage, QObject* parent, 
                                 int lum_sensibility, int lum_shadows, int lum_midtones, int lum_highlights,
                                 int chroma_sensibility, int chroma_shadows, int chroma_midtones, int chroma_highlights)
               : DImgThreadedFilter(orgImage, parent, "FilmGrain")
{
    m_lum_sensibility    = lum_sensibility;
    m_lum_shadows        = lum_shadows;
    m_lum_midtones       = lum_midtones;
    m_lum_highlights     = lum_highlights;
    
    m_chroma_sensibility = chroma_sensibility;
    m_chroma_shadows     = chroma_shadows;
    m_chroma_midtones    = chroma_midtones;
    m_chroma_highlights  = chroma_highlights;
    
    initFilter();
}

FilmGrainFilter::FilmGrainFilter(DImgThreadedFilter* parentFilter,
                                 const DImg& orgImage, const DImg& destImage,
                                 int progressBegin, int progressEnd, 
                                 int lum_sensibility, int lum_shadows, int lum_midtones, int lum_highlights,
                                 int chroma_sensibility, int chroma_shadows, int chroma_midtones, int chroma_highlights
                                 )
          : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                               parentFilter->filterName() + ": FilmGrain")
{   
    m_lum_sensibility    = lum_sensibility;
    m_lum_shadows        = lum_shadows;
    m_lum_midtones       = lum_midtones;
    m_lum_highlights     = lum_highlights;
    
    m_chroma_sensibility = chroma_sensibility;
    m_chroma_shadows     = chroma_shadows;
    m_chroma_midtones    = chroma_midtones;
    m_chroma_highlights  = chroma_highlights;
    
    filterImage();
}

void FilmGrainFilter::filterImage()
{
    // m_sensibility: 800..6400
    if (m_lum_sensibility <= 0) return;
    if (m_chroma_sensibility <= 0) return;
    
    DColor color;
    int    h, s, l;
    int    progress;

    int  width   = m_orgImage.width();
    int  height  = m_orgImage.height();
    bool sb      = m_orgImage.sixteenBit();
    int  lum_noise    = ((m_lum_sensibility   +200) / 1000) * 3 * (sb ? 256 : 1);
    int  chroma_noise = ((m_chroma_sensibility+200) / 1000) * 3 * (sb ? 256 : 1);
    
    qsrand(1); // noise will always be the same

    for (int x = 0; !m_cancel && x < width; ++x)
    {
        for (int y = 0; !m_cancel && y < height; ++y)
        {
            double lightness, hue;
            int local_lum_noise, local_chroma_noise;
            color = m_orgImage.getPixelColor(x, y);
            color.getHSL(&h, &s, &l);
            lightness = l / (sb ? 65535.0 : 255.0);
            hue       = h / (sb ? 65535.0 : 255.0);
            local_lum_noise    = interpolate(m_lum_shadows   ,m_lum_midtones   ,m_lum_highlights   ,lightness) * lum_noise   +1;
            local_chroma_noise = interpolate(m_chroma_shadows,m_chroma_midtones,m_chroma_highlights,hue      ) * chroma_noise+1;
            l = randomize(l,sb, local_lum_noise);
            h = randomize(h,sb, local_chroma_noise);
            
            color.setRGB(h, s, l, sb);
            m_destImage.setPixelColor(x, y, color);
        }

        // Update progress bar in dialog.
        progress = (int) (((double)x * 100.0) / width);

        if (progress%5 == 0)
            postProgress( progress );
    }
}

int FilmGrainFilter::randomize(int value, bool sixteenbit, int range)
{
    int nRand = (qrand() % range) - range/2.0;            
    if (!sixteenbit)
        {
            return (CLAMP0255(value+nRand));
        }
    else
        {
            return CLAMP065535(value+nRand);
        }
}

double FilmGrainFilter::interpolate(int shadows,int midtones,int highlights, double x)
{
  double s = (shadows   +100)/200.0;
  double m = (midtones  +100)/200.0; 
  double h = (highlights+100)/200.0;
  
  if (x>=0 && x <=0.5)
  {
      return (s+2*(m-s)*x);
  }
  else if (x>=0.5 && x <=1.0)
  {
      return (2*(h-m)*x+2*m-h);
  }
  else
      return 1.0;
}

}  // namespace Digikam
