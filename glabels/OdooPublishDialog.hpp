//  OdooPublishDialog.hpp
//
//  Copyright (C) 2026  Rosen Vladimirov <vladimirov.rosen@gmail.com>
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

#ifndef OdooPublishDialog_hpp
#define OdooPublishDialog_hpp


#include "merge/OdooClient.hpp"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;

namespace glabels::model
{
        class Model;
}


namespace glabels
{

        ///
        /// Publish the current label as an Odoo report (report_glabels, v19)
        ///
        /// Creates or updates an ir.actions.report of type "glabels" with the
        /// label as template file and a CSV whose first line lists the
        /// columns (field paths) the server fills when printing.
        ///
        class OdooPublishDialog : public QDialog
        {
                Q_OBJECT


                /////////////////////////////////
                // Life Cycle
                /////////////////////////////////
        public:
                OdooPublishDialog( model::Model* model, QWidget* parent = nullptr );
                ~OdooPublishDialog() override = default;


                /////////////////////////////////
                // Slots
                /////////////////////////////////
        private slots:
                void onTargetChanged( int index );
                void onPublishClicked();


                /////////////////////////////////
                // Private methods
                /////////////////////////////////
        private:
                void loadReports();
                QString uniqueReportName( merge::OdooClient& client, const QString& stem );
                void setStatus( const QString& text, bool error );


                /////////////////////////////////
                // Private data
                /////////////////////////////////
        private:
                model::Model*     mModel;
                merge::OdooSource mSource;

                QComboBox* mTargetCombo;
                QLineEdit* mNameEdit;
                QLineEdit* mColumnsEdit;
                QCheckBox* mPrintMenuCheck;
                QLabel*    mStatusLabel;
        };

}


#endif // OdooPublishDialog_hpp
