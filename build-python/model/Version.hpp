/*  Version.hpp
 *
 *  Copyright (C) 2016-2026  Jaye Evins <evins@snaught.com>
 *
 *  This file is part of gLabels-qt.
 *
 *  gLabels-qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels-qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels-qt.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef model_Version_hpp
#define model_Version_hpp


#include <QString>


namespace glabels::model
{

        namespace Version
        {
                const QString ORGANIZATION_NAME   = "glabels.org";
                const QString ORGANIZATION_DOMAIN = "glabels.org";
                const QString APPLICATION_NAME    = "glabels-qt";
                const QString DESKTOP_FILE_NAME   = "org.glabels.glabels-qt";

                const QString WEBSITE = "https://glabels.org";
                const QString BUG_WEBSITE = "https://github.com/j-evins/glabels-qt/issues";

                const int     MAJOR   = 3;
                const int     MINOR   = 99;
                const int     MICRO   = 0;

                const QString STRING  = "3.99-master636";
                const QString LONG_STRING  = "3.99-master636 (a4af368 2026-03-26)";

                QString details();
        }

}


#endif // model_Version_hpp
