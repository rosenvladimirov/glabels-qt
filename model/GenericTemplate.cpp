//  GenericTemplate.cpp
//
//  Copyright (C) 2026  Jaye Evins <evins@snaught.com>
//
//  This file is part of gLabels-qt.
//
//  gLabels-qt is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  gLabels-qt is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with gLabels-qt.  If not, see <http://www.gnu.org/licenses/>.
//


#include "GenericTemplate.hpp"

#include "FrameRect.hpp"

#include <QDebug>


namespace glabels::model
{


        Template GenericTemplate::fullPage( const Paper& paper )
        {
                Template tmplate( tr("Generic"),
                                  "FP-" + paper.id(),
                                  QString( tr( "%1 full-page labels" ) ).arg( paper.name() ),
                                  paper.id(),
                                  paper.width(),
                                  paper.height() );

                FrameRect frame( paper.width(),
                                 paper.height(),
                                 Distance::pt( 0 ),
                                 Distance::pt( 0 ),
                                 Distance::pt( 0 ) );

                Layout layout( 1,
                               1,
                               Distance::pt( 0 ),
                               Distance::pt( 0 ),
                               Distance::pt( 0 ),
                               Distance::pt( 0 ) );

                frame.addLayout( layout );
                tmplate.addFrame( frame );

                return tmplate;
        }


        Template GenericTemplate::envelope( const Paper& paper )
        {
                Template tmplate( tr("Generic"),
                                  "EN-" + paper.id(),
                                  QString( tr( "%1 envelope" ) ).arg( paper.name() ),
                                  paper.id(),
                                  paper.width(),
                                  paper.height() );

                FrameRect frame( paper.width(),
                                 paper.height(),
                                 Distance::pt( 0 ),
                                 Distance::pt( 0 ),
                                 Distance::pt( 0 ) );

                Layout layout( 1,
                               1,
                               Distance::pt( 0 ),
                               Distance::pt( 0 ),
                               Distance::pt( 0 ),
                               Distance::pt( 0 ) );

                frame.addLayout( layout );
                tmplate.addFrame( frame );

                return tmplate;
        }


}
