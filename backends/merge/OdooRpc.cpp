//  Merge/OdooRpc.cpp
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


#include "OdooRpc.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>


namespace
{
        static const QString ID = "Odoo/RPC";

        // Field types skipped when the source does not list fields explicitly
        const QStringList SKIPPED_TYPES = { "binary", "one2many", "many2many", "html",
                                            "properties", "properties_definition", "json" };
}


namespace glabels::merge
{

        ///
        /// Constructor
        ///
        OdooRpc::OdooRpc()
        {
                mId = ID;
        }


        ///
        /// Constructor
        ///
        OdooRpc::OdooRpc( const OdooRpc* merge )
                : Merge( merge ),
                  mKeys(merge->mKeys),
                  mTypes(merge->mTypes),
                  mError(merge->mError)
        {
        }


        ///
        /// Clone
        ///
        OdooRpc* OdooRpc::clone() const
        {
                return new OdooRpc( this );
        }


        ///
        /// Get ID
        ///
        QString OdooRpc::id()
        {
                return ID;
        }


        ///
        /// Create
        ///
        Merge* OdooRpc::create()
        {
                return new OdooRpc();
        }


        ///
        /// Parse a domain: JSON, or a Python-style literal as written in Odoo
        ///
        bool OdooRpc::parseDomain( const QString& text, QJsonArray& domain )
        {
                QString t = text.trimmed();
                if ( t.isEmpty() )
                {
                        domain = QJsonArray();
                        return true;
                }

                QJsonParseError error;
                QJsonDocument doc = QJsonDocument::fromJson( t.toUtf8(), &error );
                if ( error.error != QJsonParseError::NoError )
                {
                        // Python literal: ('a', '=', True) -> ["a", "=", true]
                        QString py = t;
                        py.replace( '\'', '"' ).replace( '(', '[' ).replace( ')', ']' );
                        py.replace( "True", "true" ).replace( "False", "false" ).replace( "None", "null" );
                        doc = QJsonDocument::fromJson( py.toUtf8(), &error );
                }

                if ( error.error != QJsonParseError::NoError || !doc.isArray() )
                {
                        return false;
                }

                domain = doc.array();
                return true;
        }


        ///
        /// Get key list
        ///
        QStringList OdooRpc::keys() const
        {
                return mKeys;
        }


        ///
        /// Get primary key
        ///
        QString OdooRpc::primaryKey() const
        {
                return "id";
        }


        ///
        /// Last error
        ///
        QString OdooRpc::errorString() const
        {
                return mError;
        }


        ///
        /// Open source: read fields and records
        ///
        void OdooRpc::open()
        {
                mKeys.clear();
                mTypes.clear();
                mRows = QJsonArray();
                mIndex = 0;
                mError.clear();

                OdooSource src = OdooSource::fromString( source() );
                if ( !src.isValid() )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooRpc",
                                                              "Invalid Odoo source: %1" ).arg( source() );
                        qWarning() << mError;
                        return;
                }

                QJsonArray domain;
                if ( !parseDomain( src.domain, domain ) )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooRpc",
                                                              "Invalid domain: %1" ).arg( src.domain );
                        qWarning() << mError;
                        return;
                }

                OdooClient client( src, OdooClient::apiKey( src ) );

                QList<OdooField> fields = client.fieldsGet( src.model );
                if ( !client.errorString().isEmpty() )
                {
                        mError = client.errorString();
                        qWarning() << "OdooRpc:" << mError;
                        return;
                }

                QMap<QString,OdooField> byName;
                for ( const auto& f : fields )
                {
                        byName[f.name] = f;
                }

                QStringList wanted;
                if ( src.fields.isEmpty() )
                {
                        for ( const auto& f : fields )
                        {
                                if ( !SKIPPED_TYPES.contains( f.type ) && f.name != "id" )
                                {
                                        wanted << f.name;
                                }
                        }
                }
                else
                {
                        for ( const auto& name : src.fields )
                        {
                                if ( byName.contains( name ) && name != "id" )
                                {
                                        wanted << name;
                                }
                                else if ( name != "id" )
                                {
                                        qWarning() << "OdooRpc: unknown field" << name << "on" << src.model;
                                }
                        }
                }

                mKeys << "id" << wanted;
                mTypes["id"] = "integer";
                for ( const auto& name : wanted )
                {
                        mTypes[name] = byName[name].type;
                }

                mRows = client.searchRead( src.model, domain, wanted, src.limit, src.order );
                if ( !client.errorString().isEmpty() )
                {
                        mError = client.errorString();
                        qWarning() << "OdooRpc:" << mError;
                }
        }


        ///
        /// Close source
        ///
        void OdooRpc::close()
        {
                mRows = QJsonArray();
                mIndex = 0;
        }


        ///
        /// Read next record
        ///
        Record OdooRpc::readNextRecord()
        {
                Record record;

                if ( mIndex >= mRows.size() )
                {
                        return record;
                }

                QJsonObject row = mRows[mIndex++].toObject();
                for ( const auto& key : mKeys )
                {
                        record[key] = OdooClient::valueToString( row[key], mTypes.value( key ) );
                }

                return record;
        }

}
