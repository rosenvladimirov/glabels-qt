//  OdooMergeDialog.hpp
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

#ifndef OdooMergeDialog_hpp
#define OdooMergeDialog_hpp


#include "merge/OdooClient.hpp"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QSpinBox;


namespace glabels
{

        ///
        /// Odoo merge source dialog
        ///
        class OdooMergeDialog : public QDialog
        {
                Q_OBJECT


                /////////////////////////////////
                // Life Cycle
                /////////////////////////////////
        public:
                OdooMergeDialog( const QString& source, QWidget* parent = nullptr );
                ~OdooMergeDialog() override = default;


                /////////////////////////////////
                // Properties
                /////////////////////////////////
        public:
                QString source() const;


                /////////////////////////////////
                // Slots
                /////////////////////////////////
        private slots:
                void onLoadModelsClicked();
                void onLoadFieldsClicked();
                void onAccepted();


                /////////////////////////////////
                // Private methods
                /////////////////////////////////
        private:
                merge::OdooSource currentSource() const;
                void setStatus( const QString& text, bool error );


                /////////////////////////////////
                // Private data
                /////////////////////////////////
        private:
                QLineEdit*   mUrlEdit;
                QLineEdit*   mDatabaseEdit;
                QLineEdit*   mLoginEdit;
                QLineEdit*   mApiKeyEdit;
                QCheckBox*   mRememberCheck;
                QComboBox*   mModelCombo;
                QListWidget* mFieldsList;
                QLineEdit*   mDomainEdit;
                QLineEdit*   mOrderEdit;
                QLineEdit*   mLangEdit;
                QSpinBox*    mLimitSpin;
                QLabel*      mStatusLabel;

                QStringList  mInitialFields;
        };

}


#endif // OdooMergeDialog_hpp
