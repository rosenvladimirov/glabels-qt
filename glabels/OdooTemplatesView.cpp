//  OdooTemplatesView.cpp
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


#include "OdooTemplatesView.hpp"

#include "model/Model.hpp"
#include "model/XmlLabelCreator.hpp"
#include "model/XmlLabelParser.hpp"

#include "merge/Factory.hpp"
#include "merge/Merge.hpp"
#include "merge/OdooRpc.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>


namespace
{
        const QString REPORT_MODEL = "ir.actions.report";

        // Field that only exists in report_glabels 19.0.2+ (the column contract)
        const QString CSV_FIELD = "glabels_csv_file";

        enum Column { NAME_COLUMN, MODEL_COLUMN, REPORT_NAME_COLUMN, PRINT_MENU_COLUMN, CHANGED_COLUMN };

        enum Role
        {
                ID_ROLE = Qt::UserRole,
                NAME_ROLE,
                MODEL_ROLE
        };
}


namespace glabels
{

        ///
        /// Constructor
        ///
        OdooTemplatesView::OdooTemplatesView( QWidget* parent )
                : QWidget(parent)
        {
                auto* title = new QLabel( tr("gLabels templates in Odoo") );
                QFont titleFont = title->font();
                titleFont.setBold( true );
                titleFont.setPointSizeF( titleFont.pointSizeF() * 1.3 );
                title->setFont( titleFont );

                // Server
                mConnectionCombo = new QComboBox;

                mUrlEdit = new QLineEdit;
                mUrlEdit->setPlaceholderText( "https://erp.example.com" );

                mDatabaseEdit = new QLineEdit;
                mDatabaseEdit->setPlaceholderText( tr("Optional on Odoo 19+ with one database") );

                mLoginEdit = new QLineEdit;
                mLoginEdit->setPlaceholderText( tr("Only needed for Odoo 18 and older") );

                mApiKeyEdit = new QLineEdit;
                mApiKeyEdit->setEchoMode( QLineEdit::Password );
                mApiKeyEdit->setPlaceholderText( tr("Odoo user API key") );

                mLangEdit = new QLineEdit;
                mLangEdit->setPlaceholderText( "bg_BG" );

                mRememberCheck = new QCheckBox( tr("Remember the key on this computer (never stored in the label file)") );
                mRememberCheck->setChecked( true );

                auto* loadButton = new QPushButton( tr("Load templates") );

                auto* serverForm = new QFormLayout;
                serverForm->addRow( tr("Saved servers:"), mConnectionCombo );
                serverForm->addRow( tr("Server URL:"), mUrlEdit );
                serverForm->addRow( tr("Database:"), mDatabaseEdit );
                serverForm->addRow( tr("Login:"), mLoginEdit );
                serverForm->addRow( tr("API key:"), mApiKeyEdit );
                serverForm->addRow( tr("Language:"), mLangEdit );
                serverForm->addRow( QString(), mRememberCheck );
                serverForm->addRow( QString(), loadButton );

                auto* serverBox = new QGroupBox( tr("Server") );
                serverBox->setLayout( serverForm );

                // Templates
                mTemplatesTree = new QTreeWidget;
                mTemplatesTree->setRootIsDecorated( false );
                mTemplatesTree->setAlternatingRowColors( true );
                mTemplatesTree->setSortingEnabled( true );
                mTemplatesTree->setHeaderLabels( { tr("Name"), tr("Model"), tr("Technical name"),
                                                   tr("Print menu"), tr("Last change") } );
                mTemplatesTree->header()->setSectionResizeMode( NAME_COLUMN, QHeaderView::Stretch );
                mTemplatesTree->sortByColumn( NAME_COLUMN, Qt::AscendingOrder );

                mOpenButton = new QPushButton( tr("Open for editing") );
                mOpenButton->setEnabled( false );

                mSaveButton = new QPushButton( tr("Save the label to Odoo") );
                mSaveButton->setEnabled( false );

                mCurrentLabel = new QLabel;
                mCurrentLabel->setWordWrap( true );

                auto* buttonRow = new QHBoxLayout;
                buttonRow->addWidget( mOpenButton );
                buttonRow->addWidget( mSaveButton );
                buttonRow->addStretch( 1 );

                mStatusLabel = new QLabel;
                mStatusLabel->setWordWrap( true );
                mStatusLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );

                auto* layout = new QVBoxLayout( this );
                layout->addWidget( title );
                layout->addWidget( serverBox );
                layout->addWidget( mTemplatesTree, 1 );
                layout->addLayout( buttonRow );
                layout->addWidget( mCurrentLabel );
                layout->addWidget( mStatusLabel );

                connect( mConnectionCombo, &QComboBox::currentIndexChanged, this, &OdooTemplatesView::onConnectionChanged );
                connect( loadButton, &QPushButton::clicked, this, &OdooTemplatesView::onLoadClicked );
                connect( mOpenButton, &QPushButton::clicked, this, &OdooTemplatesView::onOpenClicked );
                connect( mSaveButton, &QPushButton::clicked, this, &OdooTemplatesView::onSaveClicked );
                connect( mTemplatesTree, &QTreeWidget::itemSelectionChanged, this, &OdooTemplatesView::onSelectionChanged );
                connect( mTemplatesTree, &QTreeWidget::itemDoubleClicked, this, &OdooTemplatesView::onOpenClicked );

                loadConnections();
                updateCurrentLabel();
        }


        void OdooTemplatesView::setModel( model::Model* model )
        {
                mModel = model;
                mCurrent = OdooTemplate();
                updateCurrentLabel();
                emit currentTemplateChanged();
        }


        const OdooTemplate& OdooTemplatesView::currentTemplate() const
        {
                return mCurrent;
        }


        void OdooTemplatesView::setCurrentTemplate( const OdooTemplate& tmpl )
        {
                mCurrent = tmpl;
                updateCurrentLabel();
                emit currentTemplateChanged();
        }


        const OdooTemplate& OdooTemplatesView::openedTemplate() const
        {
                return mOpened;
        }


        ///
        /// First visit: list the templates of the remembered server right away
        ///
        void OdooTemplatesView::showEvent( QShowEvent* event )
        {
                QWidget::showEvent( event );

                if ( !mLoadedOnce && !mApiKeyEdit->text().isEmpty() && !connection().baseUrl.host().isEmpty() )
                {
                        mLoadedOnce = true;
                        QTimer::singleShot( 0, this, [this]() { loadTemplates(); } );
                }
        }


        void OdooTemplatesView::setStatus( const QString& text, bool error )
        {
                mStatusLabel->setStyleSheet( error ? "color: #c0392b;" : QString() );
                mStatusLabel->setText( text );
        }


        ///
        /// Server from the form (no model)
        ///
        merge::OdooSource OdooTemplatesView::connection() const
        {
                merge::OdooSource src;
                src.baseUrl  = merge::OdooSource::normalizedBaseUrl( QUrl::fromUserInput( mUrlEdit->text().trimmed() ) );
                src.database = mDatabaseEdit->text().trimmed();
                src.login    = mLoginEdit->text().trimmed();
                src.lang     = mLangEdit->text().trimmed();
                return src;
        }


        void OdooTemplatesView::loadConnections()
        {
                QSignalBlocker blocker( mConnectionCombo );

                mConnectionCombo->clear();
                mConnectionCombo->addItem( tr("New server"), QString() );
                for ( const auto& src : merge::OdooClient::savedConnections() )
                {
                        mConnectionCombo->addItem( src.keyLocation(), src.toString() );
                }

                blocker.unblock();
                mConnectionCombo->setCurrentIndex( mConnectionCombo->count() > 1 ? 1 : 0 );
                onConnectionChanged( mConnectionCombo->currentIndex() );
        }


        void OdooTemplatesView::onConnectionChanged( int index )
        {
                QString source = mConnectionCombo->itemData( index ).toString();
                merge::OdooSource src = merge::OdooSource::fromString( source );

                mUrlEdit->setText( source.isEmpty() ? QString() : src.baseUrl.toString() );
                mDatabaseEdit->setText( src.database );
                mLoginEdit->setText( src.login );
                mLangEdit->setText( src.lang );
                mApiKeyEdit->setText( source.isEmpty() ? QString() : merge::OdooClient::apiKey( src ) );

                mTemplatesTree->clear();
                setStatus( QString(), false );
        }


        void OdooTemplatesView::onLoadClicked()
        {
                merge::OdooSource src = connection();
                if ( src.baseUrl.host().isEmpty() || mApiKeyEdit->text().trimmed().isEmpty() )
                {
                        setStatus( tr("Server URL and API key are required."), true );
                        return;
                }

                merge::OdooClient::saveApiKey( src, mApiKeyEdit->text().trimmed(), mRememberCheck->isChecked() );

                if ( mRememberCheck->isChecked() && mConnectionCombo->findText( src.keyLocation() ) < 0 )
                {
                        QSignalBlocker blocker( mConnectionCombo );
                        mConnectionCombo->addItem( src.keyLocation(), src.toString() );
                        mConnectionCombo->setCurrentIndex( mConnectionCombo->count() - 1 );
                }

                mLoadedOnce = true;
                loadTemplates();
        }


        ///
        /// gLabels reports of the server
        ///
        void OdooTemplatesView::loadTemplates()
        {
                merge::OdooSource src = connection();

                QApplication::setOverrideCursor( Qt::WaitCursor );
                merge::OdooClient client( src, merge::OdooClient::apiKey( src ) );
                QJsonArray rows = client.searchRead( REPORT_MODEL,
                                                     QJsonArray{ QJsonArray{ "report_type", "=", "glabels" } },
                                                     { "name", "model", "report_name", "binding_model_id", "write_date" },
                                                     0, "model, name" );
                QApplication::restoreOverrideCursor();

                mTemplatesTree->clear();
                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return;
                }

                mTemplatesTree->setSortingEnabled( false );
                for ( const auto& row : rows )
                {
                        QJsonObject obj = row.toObject();
                        auto* item = new QTreeWidgetItem( mTemplatesTree );
                        item->setText( NAME_COLUMN, obj["name"].toString() );
                        item->setText( MODEL_COLUMN, obj["model"].toString() );
                        item->setText( REPORT_NAME_COLUMN, obj["report_name"].toString() );
                        item->setText( PRINT_MENU_COLUMN, obj["binding_model_id"].isArray() ? tr("Yes") : tr("No") );
                        item->setText( CHANGED_COLUMN, obj["write_date"].toString() );
                        item->setData( NAME_COLUMN, ID_ROLE, obj["id"].toInt() );
                        item->setData( NAME_COLUMN, NAME_ROLE, obj["name"].toString() );
                        item->setData( NAME_COLUMN, MODEL_ROLE, obj["model"].toString() );
                }
                mTemplatesTree->setSortingEnabled( true );
                for ( int i = MODEL_COLUMN; i < mTemplatesTree->columnCount(); i++ )
                {
                        mTemplatesTree->resizeColumnToContents( i );
                }

                setStatus( rows.isEmpty()
                           ? tr("No gLabels reports on this server (is report_glabels installed?).")
                           : tr("%1 gLabels reports.").arg( rows.size() ), false );
        }


        void OdooTemplatesView::onSelectionChanged()
        {
                mOpenButton->setEnabled( mTemplatesTree->currentItem() != nullptr );
        }


        ///
        /// report_glabels must be the v19 module with the CSV column contract
        ///
        bool OdooTemplatesView::hasColumnContract( merge::OdooClient& client )
        {
                bool found = false;
                for ( const auto& field : client.fieldsGet( REPORT_MODEL ) )
                {
                        found = found || field.name == CSV_FIELD;
                }
                if ( client.errorString().isEmpty() && !found )
                {
                        setStatus( tr("This server has no report_glabels 19.0.2 or newer "
                                      "(the report has no CSV column file)."), true );
                }
                else if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                }
                return found;
        }


        ///
        /// Download the template of the selected report and hand it over as a new label
        ///
        void OdooTemplatesView::onOpenClicked()
        {
                QTreeWidgetItem* item = mTemplatesTree->currentItem();
                if ( !item )
                {
                        return;
                }
                int reportId = item->data( NAME_COLUMN, ID_ROLE ).toInt();

                merge::OdooSource src = connection();
                merge::OdooClient client( src, merge::OdooClient::apiKey( src ) );

                QApplication::setOverrideCursor( Qt::WaitCursor );
                if ( !hasColumnContract( client ) )
                {
                        QApplication::restoreOverrideCursor();
                        return;
                }

                QJsonArray rows = client.searchRead( REPORT_MODEL,
                                                     QJsonArray{ QJsonArray{ "id", "=", reportId } },
                                                     { "name", "model", "glabels_template_file", CSV_FIELD, "write_date" },
                                                     1, QString() );
                if ( !client.errorString().isEmpty() || rows.isEmpty() )
                {
                        QApplication::restoreOverrideCursor();
                        setStatus( client.errorString().isEmpty()
                                   ? tr("The report no longer exists on the server.")
                                   : client.errorString(), true );
                        return;
                }
                QJsonObject row = rows[0].toObject();

                // Binary fields come as base64, or false when empty
                QByteArray templateData = QByteArray::fromBase64( row["glabels_template_file"].toString().toLatin1() );
                if ( templateData.isEmpty() )
                {
                        QApplication::restoreOverrideCursor();
                        setStatus( tr("The report has no template file."), true );
                        return;
                }

                model::Model* label = model::XmlLabelParser::readBuffer( templateData );
                if ( !label )
                {
                        QApplication::restoreOverrideCursor();
                        setStatus( tr("The template file of the report is not a gLabels label."), true );
                        return;
                }

                OdooTemplate tmpl;
                tmpl.connection = src;
                tmpl.reportId   = reportId;
                tmpl.name       = row["name"].toString();
                tmpl.model      = row["model"].toString();
                QByteArray csv  = QByteArray::fromBase64( row[CSV_FIELD].toString().toLatin1() );
                tmpl.columns    = merge::OdooReport::parseCsvHeader( csv );
                tmpl.writeDate  = row["write_date"].toString();

                // The CSV is the column contract; its second line holds the
                // designer's sample values
                QList<QStringList> csvRows = merge::OdooReport::parseCsv( csv );
                QStringList header = csvRows.value( 0 );
                QStringList sample = csvRows.value( 1 );
                for ( int i = 0; i < header.size() && i < sample.size(); i++ )
                {
                        tmpl.samples[ header[i].trimmed() ] = sample[i];
                }

                // Records of the report model as merge source, with exactly the
                // columns of the CSV: fields, paths through related models,
                // report_glabels values and columns of other modules
                merge::OdooSource mergeSource = src;
                mergeSource.model  = tmpl.model;
                mergeSource.fields = tmpl.columns;

                auto* odooMerge = dynamic_cast<merge::OdooRpc*>( merge::Factory::createMerge( merge::OdooRpc::id() ) );
                odooMerge->setFallbackValues( tmpl.samples );
                label->setMerge( odooMerge );
                label->merge()->setSource( mergeSource.toString() );
                QString mergeWarning = label->merge()->errorString();
                QStringList unresolved = odooMerge->unresolvedKeys();
                label->clearModified();
                QApplication::restoreOverrideCursor();

                mOpened = tmpl;
                if ( !mergeWarning.isEmpty() )
                {
                        setStatus( tr("Opened \"%1\" without merge data: %2").arg( tmpl.name, mergeWarning ), true );
                }
                else if ( !unresolved.isEmpty() )
                {
                        setStatus( tr("Opened \"%1\". Filled by Odoo when printing (shown here with the CSV sample): %2")
                                   .arg( tmpl.name, unresolved.join( ", " ) ), false );
                }
                else
                {
                        setStatus( tr("Opened \"%1\".").arg( tmpl.name ), false );
                }

                emit templateOpened( label );
        }


        void OdooTemplatesView::onSaveClicked()
        {
                saveToOdoo();
        }


        ///
        /// Write the current label back to the report it was opened from
        ///
        bool OdooTemplatesView::saveToOdoo()
        {
                if ( !mModel || !mCurrent.isValid() )
                {
                        return false;
                }

                // Only the files change; nothing translatable is written
                merge::OdooSource src = mCurrent.connection;
                src.lang.clear();
                merge::OdooClient client( src, merge::OdooClient::apiKey( mCurrent.connection ) );

                QApplication::setOverrideCursor( Qt::WaitCursor );
                QJsonArray rows = client.searchRead( REPORT_MODEL,
                                                     QJsonArray{ QJsonArray{ "id", "=", mCurrent.reportId } },
                                                     { "write_date" }, 1, QString() );
                QApplication::restoreOverrideCursor();
                if ( !client.errorString().isEmpty() || rows.isEmpty() )
                {
                        setStatus( client.errorString().isEmpty()
                                   ? tr("The report no longer exists on the server.")
                                   : client.errorString(), true );
                        return false;
                }

                if ( rows[0].toObject()["write_date"].toString() != mCurrent.writeDate )
                {
                        auto answer = QMessageBox::question( this, tr("Save to Odoo"),
                                                             tr("Report \"%1\" was changed on the server after you "
                                                                "opened it. Overwrite it with this label?").arg( mCurrent.name ) );
                        if ( answer != QMessageBox::Yes )
                        {
                                return false;
                        }
                }

                // The columns of the merge source are the contract, when it is
                // still the report model; otherwise the CSV stays as it was
                QStringList columns = mCurrent.columns;
                auto* odooMerge = dynamic_cast<merge::OdooRpc*>( mModel->merge() );
                if ( odooMerge )
                {
                        merge::OdooSource mergeSource = merge::OdooSource::fromString( odooMerge->source() );
                        if ( mergeSource.model == mCurrent.model && !mergeSource.fields.isEmpty() )
                        {
                                columns = mergeSource.fields;
                        }
                }
                if ( columns.isEmpty() )
                {
                        setStatus( tr("List at least one column."), true );
                        return false;
                }
                QStringList bad = merge::OdooReport::invalidColumns( columns );
                if ( !bad.isEmpty() )
                {
                        setStatus( tr("gLabels cannot use a field name containing ':' or '}': %1").arg( bad.join( ", " ) ), true );
                        return false;
                }

                // Header plus a sample row: the first record, or the old sample
                QStringList sample;
                for ( const auto& column : columns )
                {
                        sample << mCurrent.samples.value( column );
                }
                if ( odooMerge )
                {
                        sample = odooMerge->sampleRow( columns );
                        for ( int i = 0; i < columns.size(); i++ )
                        {
                                if ( sample[i].isEmpty() )
                                {
                                        sample[i] = mCurrent.samples.value( columns[i] );
                                }
                        }
                }
                QByteArray csvData = merge::OdooReport::csvFile( columns, { sample } );

                QByteArray templateData;
                model::XmlLabelCreator::writeBuffer( mModel, templateData );

                QJsonObject values;
                values["glabels_template_file"] = QString::fromLatin1( templateData.toBase64() );
                values[CSV_FIELD]               = QString::fromLatin1( csvData.toBase64() );

                QApplication::setOverrideCursor( Qt::WaitCursor );
                client.write( REPORT_MODEL, { mCurrent.reportId }, values );
                if ( client.errorString().isEmpty() )
                {
                        rows = client.searchRead( REPORT_MODEL,
                                                  QJsonArray{ QJsonArray{ "id", "=", mCurrent.reportId } },
                                                  { "write_date" }, 1, QString() );
                }
                QApplication::restoreOverrideCursor();

                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return false;
                }

                mCurrent.columns = columns;
                mCurrent.samples.clear();
                for ( int i = 0; i < columns.size(); i++ )
                {
                        mCurrent.samples[ columns[i] ] = sample[i];
                }
                if ( !rows.isEmpty() )
                {
                        mCurrent.writeDate = rows[0].toObject()["write_date"].toString();
                }
                mModel->clearModified();

                // New "last change" in the list, then the result
                if ( connection().keyLocation() == mCurrent.connection.keyLocation() && mTemplatesTree->topLevelItemCount() > 0 )
                {
                        loadTemplates();
                }
                setStatus( tr("Saved to report \"%1\" (id %2).").arg( mCurrent.name ).arg( mCurrent.reportId ), false );
                return true;
        }


        void OdooTemplatesView::updateCurrentLabel()
        {
                mSaveButton->setEnabled( mModel && mCurrent.isValid() );

                if ( !mModel )
                {
                        mCurrentLabel->setText( tr("No label is open. Open a template from the list to edit it.") );
                }
                else if ( mCurrent.isValid() )
                {
                        mCurrentLabel->setText( tr("The current label is report \"%1\" (id %2) on %3.")
                                                .arg( mCurrent.name ).arg( mCurrent.reportId )
                                                .arg( mCurrent.connection.keyLocation() ) );
                }
                else
                {
                        mCurrentLabel->setText( tr("The current label is not from Odoo. "
                                                   "Use File → Publish to Odoo to create a report from it.") );
                }
        }

}
