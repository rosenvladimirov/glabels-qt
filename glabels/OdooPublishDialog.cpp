//  OdooPublishDialog.cpp
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


#include "OdooPublishDialog.hpp"

#include "model/Model.hpp"
#include "model/XmlLabelCreator.hpp"

#include "merge/Merge.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>


namespace
{
        // Field that only exists in report_glabels 19.0.2+ (the column contract)
        const QString CSV_FIELD = "glabels_csv_file";
}


namespace glabels
{

        ///
        /// Constructor
        ///
        OdooPublishDialog::OdooPublishDialog( model::Model* model, QWidget* parent )
                : QDialog(parent), mModel(model)
        {
                setWindowTitle( tr("Publish to Odoo") );

                mSource = merge::OdooSource::fromString( mModel->merge()->source() );

                auto* serverLabel = new QLabel( QString( "%1 — %2" ).arg( mSource.keyLocation(), mSource.model ) );
                serverLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );

                mTargetCombo = new QComboBox;
                mTargetCombo->addItem( tr("New report"), 0 );

                mNameEdit = new QLineEdit( mModel->shortName() );

                mColumnsEdit = new QLineEdit( mModel->merge()->keys().join( ", " ) );
                mColumnsEdit->setToolTip( tr("Field paths (e.g. categ_id.name) or report_glabels values "
                                             "(creator, company, date_now, currency), separated by commas.") );

                mPrintMenuCheck = new QCheckBox( tr("Add to the Print menu of the model") );
                mPrintMenuCheck->setChecked( true );

                mStatusLabel = new QLabel;
                mStatusLabel->setWordWrap( true );
                mStatusLabel->setTextInteractionFlags( Qt::TextSelectableByMouse );

                auto* form = new QFormLayout;
                form->addRow( tr("Server:"), serverLabel );
                form->addRow( tr("Report:"), mTargetCombo );
                form->addRow( tr("Name:"), mNameEdit );
                form->addRow( tr("Columns:"), mColumnsEdit );
                form->addRow( QString(), mPrintMenuCheck );

                auto* buttons = new QDialogButtonBox( QDialogButtonBox::Cancel );
                auto* publishButton = buttons->addButton( tr("Publish"), QDialogButtonBox::AcceptRole );

                auto* layout = new QVBoxLayout( this );
                layout->addLayout( form );
                layout->addWidget( mStatusLabel );
                layout->addWidget( buttons );

                connect( mTargetCombo, &QComboBox::currentIndexChanged, this, &OdooPublishDialog::onTargetChanged );
                connect( publishButton, &QPushButton::clicked, this, &OdooPublishDialog::onPublishClicked );
                connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );

                resize( 560, sizeHint().height() );

                loadReports();
        }


        void OdooPublishDialog::setStatus( const QString& text, bool error )
        {
                mStatusLabel->setStyleSheet( error ? "color: #c0392b;" : QString() );
                mStatusLabel->setText( text );
        }


        ///
        /// Existing gLabels reports of the model
        ///
        void OdooPublishDialog::loadReports()
        {
                QApplication::setOverrideCursor( Qt::WaitCursor );
                merge::OdooClient client( mSource, merge::OdooClient::apiKey( mSource ) );
                QJsonArray rows = client.searchRead( "ir.actions.report",
                                                     QJsonArray{ QJsonArray{ "report_type", "=", "glabels" },
                                                                 QJsonArray{ "model", "=", mSource.model } },
                                                     { "name", "report_name", "binding_model_id" }, 0, "name" );
                QApplication::restoreOverrideCursor();

                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return;
                }

                for ( const auto& row : rows )
                {
                        QJsonObject obj = row.toObject();
                        mTargetCombo->addItem( QString( "%1  (%2)" ).arg( obj["name"].toString(),
                                                                          obj["report_name"].toString() ),
                                               obj["id"].toInt() );
                        // Remember whether it is in the Print menu
                        mTargetCombo->setItemData( mTargetCombo->count() - 1,
                                                   obj["binding_model_id"].isArray(), Qt::UserRole + 1 );
                        mTargetCombo->setItemData( mTargetCombo->count() - 1,
                                                   obj["name"].toString(), Qt::UserRole + 2 );
                }
        }


        void OdooPublishDialog::onTargetChanged( int index )
        {
                if ( mTargetCombo->itemData( index ).toInt() == 0 )
                {
                        mPrintMenuCheck->setChecked( true );
                        return;
                }
                mNameEdit->setText( mTargetCombo->itemData( index, Qt::UserRole + 2 ).toString() );
                mPrintMenuCheck->setChecked( mTargetCombo->itemData( index, Qt::UserRole + 1 ).toBool() );
        }


        ///
        /// report_name must be unique: add _2, _3... when taken
        ///
        QString OdooPublishDialog::uniqueReportName( merge::OdooClient& client, const QString& stem )
        {
                QJsonArray rows = client.searchRead( "ir.actions.report",
                                                     QJsonArray{ QJsonArray{ "report_name", "=like", stem + "%" } },
                                                     { "report_name" }, 0, QString() );
                QStringList taken;
                for ( const auto& row : rows )
                {
                        taken << row.toObject()["report_name"].toString();
                }

                QString name = stem;
                for ( int i = 2; taken.contains( name ); i++ )
                {
                        name = QString( "%1_%2" ).arg( stem ).arg( i );
                }
                return name;
        }


        ///
        /// Create or update the report
        ///
        void OdooPublishDialog::onPublishClicked()
        {
                QString name = mNameEdit->text().trimmed();
                if ( name.isEmpty() )
                {
                        setStatus( tr("Enter a report name."), true );
                        return;
                }

                QStringList columns = merge::OdooReport::parseColumns( mColumnsEdit->text() );
                if ( columns.isEmpty() )
                {
                        setStatus( tr("List at least one column."), true );
                        return;
                }
                QStringList bad = merge::OdooReport::invalidColumns( columns );
                if ( !bad.isEmpty() )
                {
                        setStatus( tr("gLabels cannot use a field name containing ':' or '}': %1").arg( bad.join( ", " ) ), true );
                        return;
                }

                QApplication::setOverrideCursor( Qt::WaitCursor );
                merge::OdooClient client( mSource, merge::OdooClient::apiKey( mSource ) );

                // report_glabels must be the v19 module with the CSV column contract
                bool hasContract = false;
                for ( const auto& field : client.fieldsGet( "ir.actions.report" ) )
                {
                        hasContract = hasContract || field.name == CSV_FIELD;
                }
                if ( !client.errorString().isEmpty() || !hasContract )
                {
                        QApplication::restoreOverrideCursor();
                        setStatus( client.errorString().isEmpty()
                                   ? tr("This server has no report_glabels 19.0.2 or newer "
                                        "(the report has no CSV column file).")
                                   : client.errorString(), true );
                        return;
                }

                QByteArray templateData;
                model::XmlLabelCreator::writeBuffer( mModel, templateData );

                QJsonObject values;
                values["name"]                      = name;
                values["glabels_template_file"]     = QString::fromLatin1( templateData.toBase64() );
                values["glabels_template_filename"] = name + ".glabels";
                values["glabels_csv_file"]          = QString::fromLatin1( merge::OdooReport::csvHeader( columns ).toBase64() );
                values["glabels_csv_filename"]      = name + ".csv";

                if ( mPrintMenuCheck->isChecked() )
                {
                        int modelId = client.modelId( mSource.model );
                        if ( modelId == 0 )
                        {
                                QApplication::restoreOverrideCursor();
                                setStatus( client.errorString().isEmpty()
                                           ? tr("Model %1 not found on the server.").arg( mSource.model )
                                           : client.errorString(), true );
                                return;
                        }
                        values["binding_model_id"] = modelId;
                        values["binding_type"]     = "report";
                }
                else
                {
                        values["binding_model_id"] = false;
                }

                int reportId = mTargetCombo->currentData().toInt();
                bool created = ( reportId == 0 );
                if ( created )
                {
                        values["model"]       = mSource.model;
                        values["report_type"] = "glabels";
                        values["report_name"] = uniqueReportName( client, merge::OdooReport::reportNameStem( mSource.model, name ) );
                        if ( client.errorString().isEmpty() )
                        {
                                reportId = client.create( "ir.actions.report", values );
                        }
                }
                else
                {
                        client.write( "ir.actions.report", { reportId }, values );
                }
                QApplication::restoreOverrideCursor();

                if ( !client.errorString().isEmpty() )
                {
                        setStatus( client.errorString(), true );
                        return;
                }

                QMessageBox::information( this, tr("Publish to Odoo"),
                                          created
                                          ? tr("Report \"%1\" created (id %2).").arg( name ).arg( reportId )
                                          : tr("Report \"%1\" updated (id %2).").arg( name ).arg( reportId ) );
                accept();
        }

}
