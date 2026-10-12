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
#include <QDate>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QSet>

#include <functional>


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
                  mUnresolved(merge->mUnresolved),
                  mFallback(merge->mFallback),
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


        void OdooRpc::setFallbackValues( const QMap<QString,QString>& values )
        {
                mFallback = values;
        }


        QStringList OdooRpc::unresolvedKeys() const
        {
                return mUnresolved;
        }


        QStringList OdooRpc::sampleRow( const QStringList& columns ) const
        {
                const QList<Record>& records = recordList();

                QStringList row;
                for ( const auto& column : columns )
                {
                        if ( mUnresolved.contains( column ) || records.isEmpty() )
                        {
                                row << mFallback.value( column );
                        }
                        else
                        {
                                row << records.first().value( column );
                        }
                }
                return row;
        }


        ///
        /// Open source: read fields and records
        ///
        void OdooRpc::open()
        {
                mKeys.clear();
                mUnresolved.clear();
                mRecords.clear();
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

                // fields_get once per model on the way
                QMap<QString,QMap<QString,OdooField>> fieldCache;
                auto fieldsOf = [&]( const QString& model ) -> const QMap<QString,OdooField>&
                {
                        if ( !fieldCache.contains( model ) )
                        {
                                QMap<QString,OdooField> byName;
                                for ( const auto& f : client.fieldsGet( model ) )
                                {
                                        byName[f.name] = f;
                                }
                                fieldCache[model] = byName;
                        }
                        return fieldCache[model];
                };

                const QMap<QString,OdooField> baseFields = fieldsOf( src.model );
                if ( !client.errorString().isEmpty() )
                {
                        mError = client.errorString();
                        qWarning() << "OdooRpc:" << mError;
                        return;
                }

                // Columns: as listed, or all simple fields
                QStringList columns;
                if ( src.fields.isEmpty() )
                {
                        for ( const auto& f : baseFields )
                        {
                                if ( !SKIPPED_TYPES.contains( f.type ) && f.name != "id" )
                                {
                                        columns << f.name;
                                }
                        }
                }
                else
                {
                        for ( const auto& column : src.fields )
                        {
                                if ( column != "id" && !columns.contains( column ) )
                                {
                                        columns << column;
                                }
                        }
                }
                mKeys << "id" << columns;

                // Plain stored-or-not fields come with search_read, paths are followed below
                QStringList direct;
                for ( const auto& column : columns )
                {
                        if ( baseFields.contains( column ) && !baseFields[column].isToMany() )
                        {
                                direct << column;
                        }
                }

                QJsonArray rows = client.searchRead( src.model, domain, direct.isEmpty() ? QStringList{ "id" } : direct,
                                                     src.limit, src.order );
                if ( !client.errorString().isEmpty() )
                {
                        mError = client.errorString();
                        qWarning() << "OdooRpc:" << mError;
                        return;
                }

                QList<QList<int>> baseIds;
                for ( const auto& row : rows )
                {
                        baseIds << QList<int>{ row.toObject()["id"].toInt() };
                }

                // Text per row for a path, starting from records of a model
                // (several per row after a to-many hop); joined like report_glabels
                std::function<bool( const QString&, const QList<QList<int>>&, QStringList, QStringList& )> follow;
                follow = [&]( const QString& model, const QList<QList<int>>& idsPerRow,
                              QStringList path, QStringList& out ) -> bool
                {
                        const QMap<QString,OdooField>& modelFields = fieldsOf( model );
                        if ( !modelFields.contains( path.first() ) )
                        {
                                return false;
                        }
                        OdooField field = modelFields[path.first()];

                        // A relation at the end of the path prints its names
                        if ( path.size() == 1 && field.isToMany() )
                        {
                                path << "display_name";
                        }
                        if ( path.size() > 1 && !field.isRelational() )
                        {
                                return false;
                        }

                        QSet<int> unique;
                        for ( const auto& ids : idsPerRow )
                        {
                                for ( int id : ids ) unique.insert( id );
                        }
                        QMap<int,QJsonValue> values;
                        if ( !unique.isEmpty() )
                        {
                                for ( const auto& rec : client.read( model, unique.values(), { field.name } ) )
                                {
                                        QJsonObject obj = rec.toObject();
                                        values[obj["id"].toInt()] = obj[field.name];
                                }
                                if ( !client.errorString().isEmpty() )
                                {
                                        return false;
                                }
                        }

                        if ( path.size() == 1 )
                        {
                                out.clear();
                                for ( const auto& ids : idsPerRow )
                                {
                                        QStringList texts;
                                        for ( int id : ids )
                                        {
                                                QString text = OdooClient::valueToString( values.value( id ), field.type );
                                                if ( !text.isEmpty() ) texts << text;
                                        }
                                        out << texts.join( ", " );
                                }
                                return true;
                        }

                        QList<QList<int>> nextIds;
                        for ( const auto& ids : idsPerRow )
                        {
                                QList<int> next;
                                for ( int id : ids )
                                {
                                        QJsonValue value = values.value( id );
                                        if ( field.isToMany() )
                                        {
                                                for ( const auto& v : value.toArray() ) next << v.toInt();
                                        }
                                        else if ( value.isArray() && !value.toArray().isEmpty() )
                                        {
                                                next << value.toArray().first().toInt();
                                        }
                                }
                                nextIds << next;
                        }
                        return follow( field.relation, nextIds, path.mid( 1 ), out );
                };

                QMap<QString,QStringList> followed;
                for ( const auto& column : columns )
                {
                        if ( direct.contains( column ) )
                        {
                                continue;
                        }
                        QStringList out;
                        if ( !rows.isEmpty() && follow( src.model, baseIds, column.split( '.' ), out ) )
                        {
                                followed[column] = out;
                        }
                        else if ( rows.isEmpty() && baseFields.contains( column.section( '.', 0, 0 ) ) )
                        {
                                followed[column] = QStringList();
                        }
                        else
                        {
                                if ( !client.errorString().isEmpty() )
                                {
                                        qWarning() << "OdooRpc:" << column << client.errorString();
                                }
                                mUnresolved << column;
                        }
                }

                for ( int i = 0; i < rows.size(); i++ )
                {
                        QJsonObject row = rows[i].toObject();
                        Record record;
                        record["id"] = QString::number( row["id"].toInt() );
                        for ( const auto& column : columns )
                        {
                                if ( direct.contains( column ) )
                                {
                                        record[column] = OdooClient::valueToString( row[column], baseFields[column].type );
                                }
                                else if ( followed.contains( column ) )
                                {
                                        record[column] = followed[column].value( i );
                                }
                                else if ( mFallback.contains( column ) )
                                {
                                        record[column] = mFallback[column];
                                }
                                else if ( column == "date_now" )
                                {
                                        record[column] = QLocale().toString( QDate::currentDate(), QLocale::ShortFormat );
                                }
                                else
                                {
                                        // Visible in the editor, so the layout can be checked
                                        record[column] = "[" + column + "]";
                                }
                        }
                        mRecords << record;
                }
        }


        ///
        /// Close source
        ///
        void OdooRpc::close()
        {
                mRecords.clear();
                mIndex = 0;
        }


        ///
        /// Read next record
        ///
        Record OdooRpc::readNextRecord()
        {
                if ( mIndex >= mRecords.size() )
                {
                        return Record();
                }
                return mRecords[mIndex++];
        }

}
