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
#include <QHeaderView>
#include <QTreeWidget>
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

                // Fields of the model; relations open into the fields of the
                // related model, so paths like product_tmpl_id.name can be picked
                mFieldsTree = new QTreeWidget;
                mFieldsTree->setMinimumHeight( 220 );
                mFieldsTree->setHeaderLabels( { tr("Field"), tr("Label"), tr("Type") } );
                mFieldsTree->header()->setSectionResizeMode( 0, QHeaderView::ResizeToContents );
                auto* loadFieldsButton = new QPushButton( tr("Load fields") );

                // The columns themselves: their order, plus values that are not
                // fields (company, date_now, columns of other modules like price)
                mColumnsEdit = new QLineEdit( src.fields.join( ", " ) );
                mColumnsEdit->setPlaceholderText( "product_tmpl_id.name, default_code, price, date_now" );
                mColumnsEdit->setToolTip( tr("Report columns in order, separated by commas: fields, field paths "
                                             "through related models, report_glabels values (creator, company, "
                                             "date_now, currency) or columns of other modules.") );

                auto* fieldsBox = new QVBoxLayout;
                fieldsBox->addWidget( mFieldsTree );
                fieldsBox->addWidget( loadFieldsButton, 0, Qt::AlignLeft );
                fieldsBox->addWidget( new QLabel( tr("Columns (empty = all simple fields):") ) );
                fieldsBox->addWidget( mColumnsEdit );

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
                connect( mFieldsTree, &QTreeWidget::itemExpanded, this, &OdooMergeDialog::onFieldExpanded );
                connect( mFieldsTree, &QTreeWidget::itemChanged, this, &OdooMergeDialog::onFieldChanged );
                connect( mColumnsEdit, &QLineEdit::editingFinished, this, &OdooMergeDialog::onColumnsEdited );
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

                src.fields   = columns();

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


        QStringList OdooMergeDialog::columns() const
        {
                return merge::OdooReport::parseColumns( mColumnsEdit->text() );
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

                mFieldsTree->clear();
                addFieldItems( mFieldsTree->invisibleRootItem(), src.model, QString() );
                if ( mFieldsTree->topLevelItemCount() > 0 )
                {
                        setStatus( tr("%1 fields loaded from %2. Open a relation to use the fields "
                                      "of the related model.").arg( mFieldsTree->topLevelItemCount() ).arg( src.model ), false );
                }
        }


        ///
        /// Field items of a model under parent; relations get a placeholder
        /// child so they can be opened
        ///
        void OdooMergeDialog::addFieldItems( QTreeWidgetItem* parent, const QString& model, const QString& prefix )
        {
                merge::OdooSource src = currentSource();

                QApplication::setOverrideCursor( Qt::WaitCursor );
                merge::OdooClient client( src, mApiKeyEdit->text().trimmed() );
                QList<merge::OdooField> fields = client.fieldsGet( model );
                QApplication::restoreOverrideCursor();

                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return;
                }

                QStringList selected = columns();

                mSyncing = true;
                for ( const auto& f : fields )
                {
                        QString path = prefix.isEmpty() ? f.name : prefix + "." + f.name;
                        auto* item = new QTreeWidgetItem( parent );
                        item->setText( 0, f.name );
                        item->setText( 1, f.label );
                        item->setText( 2, f.isRelational() ? QString( "%1 → %2" ).arg( f.type, f.relation ) : f.type );
                        item->setData( 0, Qt::UserRole, path );
                        item->setData( 0, Qt::UserRole + 1, f.isRelational() ? f.relation : QString() );
                        item->setFlags( item->flags() | Qt::ItemIsUserCheckable );
                        item->setCheckState( 0, selected.contains( path ) ? Qt::Checked : Qt::Unchecked );
                        if ( f.isRelational() )
                        {
                                new QTreeWidgetItem( item );   // placeholder, replaced on expand
                        }
                }
                mSyncing = false;
        }


        void OdooMergeDialog::onFieldExpanded( QTreeWidgetItem* item )
        {
                QString relation = item->data( 0, Qt::UserRole + 1 ).toString();
                if ( relation.isEmpty() || item->childCount() != 1 || !item->child( 0 )->text( 0 ).isEmpty() )
                {
                        return;
                }
                delete item->takeChild( 0 );
                addFieldItems( item, relation, item->data( 0, Qt::UserRole ).toString() );
        }


        ///
        /// Checking a field adds its path to the columns, unchecking removes it
        ///
        void OdooMergeDialog::onFieldChanged( QTreeWidgetItem* item, int column )
        {
                if ( mSyncing || column != 0 )
                {
                        return;
                }

                QString path = item->data( 0, Qt::UserRole ).toString();
                QStringList list = columns();
                if ( item->checkState( 0 ) == Qt::Checked && !list.contains( path ) )
                {
                        list << path;
                }
                else if ( item->checkState( 0 ) != Qt::Checked )
                {
                        list.removeAll( path );
                }
                mColumnsEdit->setText( list.join( ", " ) );
        }


        void OdooMergeDialog::onColumnsEdited()
        {
                mSyncing = true;
                syncChecks( mFieldsTree->invisibleRootItem(), columns() );
                mSyncing = false;
        }


        void OdooMergeDialog::syncChecks( QTreeWidgetItem* item, const QStringList& columns )
        {
                for ( int i = 0; i < item->childCount(); i++ )
                {
                        QTreeWidgetItem* child = item->child( i );
                        QString path = child->data( 0, Qt::UserRole ).toString();
                        if ( !path.isEmpty() )
                        {
                                child->setCheckState( 0, columns.contains( path ) ? Qt::Checked : Qt::Unchecked );
                        }
                        syncChecks( child, columns );
                }
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

                QStringList bad = merge::OdooReport::invalidColumns( src.fields );
                if ( !bad.isEmpty() )
                {
                        setStatus( tr("gLabels cannot use a field name containing ':' or '}': %1").arg( bad.join( ", " ) ), true );
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
