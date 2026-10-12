//  OdooTemplatesView.hpp
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

#ifndef OdooTemplatesView_hpp
#define OdooTemplatesView_hpp


#include "merge/OdooClient.hpp"

#include <QMap>
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTreeWidget;

namespace glabels::model
{
        class Model;
}


namespace glabels
{

        ///
        /// gLabels report in Odoo the current label was opened from
        ///
        struct OdooTemplate
        {
                merge::OdooSource connection;   // server, database, login, language
                int               reportId{ 0 };
                QString           name;
                QString           model;
                QStringList       columns;      // CSV column contract
                QMap<QString,QString> samples;  // sample row of the CSV, per column
                QString           writeDate;    // to notice changes made by others

                bool isValid() const { return reportId != 0; }
        };


        ///
        /// Odoo page: gLabels templates (report_glabels reports) of a server
        ///
        /// Lists the reports, opens one for editing and writes the edited
        /// label back to the same report.
        ///
        class OdooTemplatesView : public QWidget
        {
                Q_OBJECT


                /////////////////////////////////
                // Life Cycle
                /////////////////////////////////
        public:
                OdooTemplatesView( QWidget* parent = nullptr );
                ~OdooTemplatesView() override = default;


                /////////////////////////////////
                // Public methods
                /////////////////////////////////
        public:
                void setModel( model::Model* model );

                const OdooTemplate& currentTemplate() const;
                void setCurrentTemplate( const OdooTemplate& tmpl );

                // Template opened last, handed over with templateOpened()
                const OdooTemplate& openedTemplate() const;

                bool saveToOdoo();


                /////////////////////////////////
                // Signals
                /////////////////////////////////
        signals:
                void templateOpened( model::Model* model );
                void currentTemplateChanged();


                /////////////////////////////////
                // Events
                /////////////////////////////////
        protected:
                void showEvent( QShowEvent* event ) override;


                /////////////////////////////////
                // Slots
                /////////////////////////////////
        private slots:
                void onConnectionChanged( int index );
                void onLoadClicked();
                void onOpenClicked();
                void onSaveClicked();
                void onSelectionChanged();


                /////////////////////////////////
                // Private methods
                /////////////////////////////////
        private:
                merge::OdooSource connection() const;
                bool hasColumnContract( merge::OdooClient& client );
                void loadConnections();
                void loadTemplates();
                void updateCurrentLabel();
                void setStatus( const QString& text, bool error );


                /////////////////////////////////
                // Private data
                /////////////////////////////////
        private:
                model::Model* mModel{ nullptr };
                OdooTemplate  mCurrent;
                OdooTemplate  mOpened;
                bool          mLoadedOnce{ false };

                QComboBox*   mConnectionCombo;
                QLineEdit*   mUrlEdit;
                QLineEdit*   mDatabaseEdit;
                QLineEdit*   mLoginEdit;
                QLineEdit*   mApiKeyEdit;
                QLineEdit*   mLangEdit;
                QCheckBox*   mRememberCheck;
                QTreeWidget* mTemplatesTree;
                QPushButton* mOpenButton;
                QPushButton* mSaveButton;
                QLabel*      mCurrentLabel;
                QLabel*      mStatusLabel;
        };

}


#endif // OdooTemplatesView_hpp
