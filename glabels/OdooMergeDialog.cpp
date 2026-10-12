//  OdooMergeDialog.cpp
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


#include "OdooMergeDialog.hpp"

#include "merge/OdooRpc.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>


namespace glabels
{

        ///
        /// Constructor
        ///
        OdooMergeDialog::OdooMergeDialog( const QString& source, QWidget* parent )
                : QDialog(parent)
        {
                setWindowTitle( tr("Odoo merge source") );

                merge::OdooSource src = merge::OdooSource::fromString( source );
                mInitialFields = src.fields;

                mUrlEdit = new QLineEdit( src.baseUrl.toString() );
                mUrlEdit->setPlaceholderText( "https://erp.example.com" );

                mDatabaseEdit = new QLineEdit( src.database );
                mDatabaseEdit->setPlaceholderText( tr("Optional on Odoo 19+ with one database") );

                mLoginEdit = new QLineEdit( src.login );
                mLoginEdit->setPlaceholderText( tr("Only needed for Odoo 18 and older") );

                mApiKeyEdit = new QLineEdit( source.isEmpty() ? QString() : merge::OdooClient::apiKey( src ) );
                mApiKeyEdit->setEchoMode( QLineEdit::Password );
                mApiKeyEdit->setPlaceholderText( tr("Odoo user API key") );

                mRememberCheck = new QCheckBox( tr("Remember the key on this computer (never stored in the label file)") );
                mRememberCheck->setChecked( true );

                mModelCombo = new QComboBox;
                mModelCombo->setEditable( true );
                mModelCombo->setEditText( src.model );
                mModelCombo->setInsertPolicy( QComboBox::NoInsert );
                auto* loadModelsButton = new QPushButton( tr("Load models") );

                auto* modelRow = new QHBoxLayout;
                modelRow->addWidget( mModelCombo, 1 );
                modelRow->addWidget( loadModelsButton );

                mFieldsList = new QListWidget;
                mFieldsList->setMinimumHeight( 180 );
                for ( const auto& name : src.fields )
                {
                        auto* item = new QListWidgetItem( name, mFieldsList );
                        item->setData( Qt::UserRole, name );
                        item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
                        item->setCheckState( Qt::Checked );
                }
                auto* loadFieldsButton = new QPushButton( tr("Load fields") );

                auto* fieldsBox = new QVBoxLayout;
                fieldsBox->addWidget( mFieldsList );
                fieldsBox->addWidget( new QLabel( tr("No field checked = all simple fields") ) );
                fieldsBox->addWidget( loadFieldsButton, 0, Qt::AlignLeft );

                mDomainEdit = new QLineEdit( src.domain );
                mDomainEdit->setPlaceholderText( "[[\"product_id.default_code\", \"=\", \"PR6030\"]]" );

                mOrderEdit = new QLineEdit( src.order );
                mOrderEdit->setPlaceholderText( "name asc" );

                mLangEdit = new QLineEdit( src.lang );
                mLangEdit->setPlaceholderText( "bg_BG" );

                mLimitSpin = new QSpinBox;
                mLimitSpin->setRange( 0, 100000 );
                mLimitSpin->setSpecialValueText( tr("No limit") );
                mLimitSpin->setValue( src.limit );

                mStatusLabel = new QLabel;
                mStatusLabel->setWordWrap( true );
                mStatusLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );

                auto* form = new QFormLayout;
                form->addRow( tr("Server URL:"), mUrlEdit );
                form->addRow( tr("Database:"), mDatabaseEdit );
                form->addRow( tr("Login:"), mLoginEdit );
                form->addRow( tr("API key:"), mApiKeyEdit );
                form->addRow( QString(), mRememberCheck );
                form->addRow( tr("Model:"), modelRow );
                form->addRow( tr("Fields:"), fieldsBox );
                form->addRow( tr("Domain:"), mDomainEdit );
                form->addRow( tr("Order:"), mOrderEdit );
                form->addRow( tr("Language:"), mLangEdit );
                form->addRow( tr("Limit:"), mLimitSpin );

                auto* buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

                auto* layout = new QVBoxLayout( this );
                layout->addLayout( form );
                layout->addWidget( mStatusLabel );
                layout->addWidget( buttons );

                if ( !qEnvironmentVariableIsEmpty( "GLABELS_ODOO_API_KEY" ) )
                {
                        setStatus( tr("GLABELS_ODOO_API_KEY is set and overrides the stored key."), false );
                }

                connect( loadModelsButton, &QPushButton::clicked, this, &OdooMergeDialog::onLoadModelsClicked );
                connect( loadFieldsButton, &QPushButton::clicked, this, &OdooMergeDialog::onLoadFieldsClicked );
                connect( buttons, &QDialogButtonBox::accepted, this, &OdooMergeDialog::onAccepted );
                connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );

                resize( 560, sizeHint().height() );
        }


        ///
        /// Source built from the dialog
        ///
        QString OdooMergeDialog::source() const
        {
                return currentSource().toString();
        }


        merge::OdooSource OdooMergeDialog::currentSource() const
        {
                merge::OdooSource src;
                src.baseUrl  = merge::OdooSource::normalizedBaseUrl( QUrl::fromUserInput( mUrlEdit->text().trimmed() ) );
                src.database = mDatabaseEdit->text().trimmed();
                src.login    = mLoginEdit->text().trimmed();
                src.model    = mModelCombo->currentText().section( ' ', 0, 0 ).trimmed();
                src.domain   = mDomainEdit->text().trimmed().isEmpty() ? "[]" : mDomainEdit->text().trimmed();
                src.order    = mOrderEdit->text().trimmed();
                src.lang     = mLangEdit->text().trimmed();
                src.limit    = mLimitSpin->value();

                for ( int i = 0; i < mFieldsList->count(); i++ )
                {
                        QListWidgetItem* item = mFieldsList->item( i );
                        if ( item->checkState() == Qt::Checked )
                        {
                                src.fields << item->data( Qt::UserRole ).toString();
                        }
                }

                return src;
        }


        void OdooMergeDialog::setStatus( const QString& text, bool error )
        {
                mStatusLabel->setStyleSheet( error ? "color: #c0392b;" : QString() );
                mStatusLabel->setText( text );
        }


        ///
        /// Load models from ir.model
        ///
        void OdooMergeDialog::onLoadModelsClicked()
        {
                merge::OdooSource src = currentSource();
                if ( !src.baseUrl.isValid() || src.baseUrl.host().isEmpty() )
                {
                        setStatus( tr("Enter the server URL first."), true );
                        return;
                }

                QApplication::setOverrideCursor( Qt::WaitCursor );
                merge::OdooClient client( src, mApiKeyEdit->text().trimmed() );
                QJsonArray rows = client.searchRead( "ir.model",
                                                     QJsonArray{ QJsonArray{ "transient", "=", false } },
                                                     { "model", "name" }, 0, "model" );
                QApplication::restoreOverrideCursor();

                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return;
                }

                QString current = mModelCombo->currentText();
                mModelCombo->clear();
                for ( const auto& row : rows )
                {
                        QJsonObject obj = row.toObject();
                        mModelCombo->addItem( obj["model"].toString() + "  (" + obj["name"].toString() + ")" );
                }
                mModelCombo->setEditText( current );
                setStatus( tr("%1 models loaded.").arg( rows.size() ), false );
        }


        ///
        /// Load fields of the selected model
        ///
        void OdooMergeDialog::onLoadFieldsClicked()
        {
                merge::OdooSource src = currentSource();
                if ( !src.isValid() )
                {
                        setStatus( tr("Enter the server URL and the model first."), true );
                        return;
                }

                QStringList checked = src.fields.isEmpty() ? mInitialFields : src.fields;

                QApplication::setOverrideCursor( Qt::WaitCursor );
                merge::OdooClient client( src, mApiKeyEdit->text().trimmed() );
                QList<merge::OdooField> fields = client.fieldsGet( src.model );
                QApplication::restoreOverrideCursor();

                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return;
                }

                mFieldsList->clear();
                for ( const auto& f : fields )
                {
                        auto* item = new QListWidgetItem( QString( "%1 — %2 (%3)" ).arg( f.name, f.label, f.type ),
                                                          mFieldsList );
                        item->setData( Qt::UserRole, f.name );
                        item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
                        item->setCheckState( checked.contains( f.name ) ? Qt::Checked : Qt::Unchecked );
                }
                setStatus( tr("%1 fields loaded from %2.").arg( fields.size() ).arg( src.model ), false );
        }


        ///
        /// Validate and store the key
        ///
        void OdooMergeDialog::onAccepted()
        {
                merge::OdooSource src = currentSource();
                if ( !src.isValid() )
                {
                        setStatus( tr("Server URL and model are required."), true );
                        return;
                }

                QJsonArray domain;
                if ( !merge::OdooRpc::parseDomain( src.domain, domain ) )
                {
                        setStatus( tr("The domain is not a valid list."), true );
                        return;
                }

                merge::OdooClient::saveApiKey( src, mApiKeyEdit->text().trimmed(), mRememberCheck->isChecked() );

                accept();
        }

}
