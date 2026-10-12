//  Merge/OdooClient.cpp
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


#include "OdooClient.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QTimer>
#include <QRegularExpression>
#include <QUrlQuery>

#include <algorithm>
#include <memory>


namespace
{
        const QString ENV_API_KEY = "GLABELS_ODOO_API_KEY";

        // Keys entered without "remember": kept for this process only
        QMap<QString,QString>& sessionKeys()
        {
                static QMap<QString,QString> keys;
                return keys;
        }


        // Characters the gLabels field parser does not accept in a field name
        // (model/SubstitutionField.cpp); report_glabels rejects them too.
        const QString FORBIDDEN_CHARS = ":}";


        QSettings* keyStore()
        {
                return new QSettings( QSettings::IniFormat, QSettings::UserScope,
                                      "glabels.org", "odoo-keys" );
        }


        QString errorFromJson( const QJsonObject& obj )
        {
                // JSON-2 error: { "name": ..., "message": ... }
                // JSON-RPC error: { "error": { "message": ..., "data": { "message": ... } } }
                if ( obj.contains( "error" ) )
                {
                        QJsonObject err = obj["error"].toObject();
                        QString msg = err["data"].toObject()["message"].toString();
                        return msg.isEmpty() ? err["message"].toString() : msg;
                }
                QString msg = obj["message"].toString();
                return msg.isEmpty() ? obj["name"].toString() : msg;
        }
}


namespace glabels::merge
{

        int OdooClient::timeoutMs = 30000;


        ///
        /// Source is usable
        ///
        bool OdooSource::isValid() const
        {
                return baseUrl.isValid() && !baseUrl.host().isEmpty() && !model.isEmpty();
        }


        ///
        /// Serialize source
        ///
        /// Every value is fully percent-encoded, so domains with '&', '=' or
        /// '+' survive the round trip through the XML "src" attribute.
        ///
        QString OdooSource::toString() const
        {
                QStringList items;
                auto add = [&items]( const QString& key, const QString& value )
                {
                        items << key + "=" + QString::fromLatin1( QUrl::toPercentEncoding( value ) );
                };

                if ( !database.isEmpty() ) add( "db", database );
                if ( !login.isEmpty() )    add( "login", login );
                add( "model", model );
                if ( !fields.isEmpty() )   add( "fields", fields.join( ',' ) );
                if ( !domain.isEmpty() && domain != "[]" ) add( "domain", domain );
                if ( !order.isEmpty() )    add( "order", order );
                if ( !lang.isEmpty() )     add( "lang", lang );
                add( "limit", QString::number( limit ) );

                return normalizedBaseUrl( baseUrl ).toString( QUrl::FullyEncoded ) + "/?" + items.join( '&' );
        }


        ///
        /// Parse source
        ///
        OdooSource OdooSource::fromString( const QString& source )
        {
                OdooSource s;
                QUrl url( source );
                QUrlQuery query( url );

                auto value = [&query]( const QString& key )
                {
                        return query.queryItemValue( key, QUrl::FullyDecoded );
                };

                s.database = value( "db" );
                s.login    = value( "login" );
                s.model    = value( "model" );
                s.order    = value( "order" );
                s.lang     = value( "lang" );

                QString fields = value( "fields" );
                if ( !fields.isEmpty() )
                {
                        s.fields = fields.split( ',', Qt::SkipEmptyParts );
                        for ( auto& f : s.fields ) f = f.trimmed();
                }

                QString domain = value( "domain" );
                if ( !domain.isEmpty() ) s.domain = domain;

                bool ok = false;
                int limit = value( "limit" ).toInt( &ok );
                if ( ok ) s.limit = limit;

                s.baseUrl = normalizedBaseUrl( url );
                return s;
        }


        ///
        /// Base URL without query, fragment, credentials and trailing slash,
        /// so "https://erp/" and "https://erp" are the same server
        ///
        QUrl OdooSource::normalizedBaseUrl( const QUrl& url )
        {
                QUrl base = url.adjusted( QUrl::RemoveQuery | QUrl::RemoveFragment |
                                          QUrl::RemoveUserInfo | QUrl::StripTrailingSlash );
                if ( base.path() == "/" )
                {
                        base.setPath( QString() );
                }
                return base;
        }


        ///
        /// Key under which the API key is stored: origin + database
        ///
        QString OdooSource::keyLocation() const
        {
                QString origin = normalizedBaseUrl( baseUrl ).toString();
                return database.isEmpty() ? origin : origin + "#" + database;
        }


        ///
        /// Constructor
        ///
        OdooClient::OdooClient( const OdooSource& source, const QString& apiKey )
                : mSource(source), mApiKey(apiKey)
        {
        }


        QString OdooClient::errorString() const
        {
                return mError;
        }


        ///
        /// Lookup API key: environment overrides the per-user key store
        ///
        QString OdooClient::apiKey( const OdooSource& source )
        {
                QString env = qEnvironmentVariable( ENV_API_KEY.toUtf8().constData() );
                if ( !env.isEmpty() )
                {
                        return env;
                }

                if ( sessionKeys().contains( source.keyLocation() ) )
                {
                        return sessionKeys()[ source.keyLocation() ];
                }

                std::unique_ptr<QSettings> settings( keyStore() );
                settings->beginGroup( "keys" );
                QString encoded = QString::fromUtf8( QUrl::toPercentEncoding( source.keyLocation() ) );
                return settings->value( encoded ).toString();
        }


        ///
        /// Store API key for this process, and optionally in the per-user
        /// key store (owner read/write only)
        ///
        void OdooClient::saveApiKey( const OdooSource& source, const QString& apiKey, bool persist )
        {
                sessionKeys()[ source.keyLocation() ] = apiKey;
                if ( !persist )
                {
                        return;
                }

                std::unique_ptr<QSettings> settings( keyStore() );
                QString encoded = QString::fromUtf8( QUrl::toPercentEncoding( source.keyLocation() ) );

                // The connection itself, so it can be offered again without the model
                OdooSource connection;
                connection.baseUrl  = OdooSource::normalizedBaseUrl( source.baseUrl );
                connection.database = source.database;
                connection.login    = source.login;
                connection.lang     = source.lang;

                for ( const QString& group : { QString( "keys" ), QString( "connections" ) } )
                {
                        settings->beginGroup( group );
                        if ( apiKey.isEmpty() )
                        {
                                settings->remove( encoded );
                        }
                        else
                        {
                                settings->setValue( encoded, group == "keys" ? apiKey : connection.toString() );
                        }
                        settings->endGroup();
                }
                settings->sync();

                QFile::setPermissions( settings->fileName(), QFile::ReadOwner | QFile::WriteOwner );
        }


        ///
        /// Remembered servers, sorted by location
        ///
        QList<OdooSource> OdooClient::savedConnections()
        {
                QList<OdooSource> connections;

                std::unique_ptr<QSettings> settings( keyStore() );
                settings->beginGroup( "connections" );
                QStringList locations = settings->childKeys();
                std::sort( locations.begin(), locations.end() );
                for ( const auto& location : locations )
                {
                        OdooSource connection = OdooSource::fromString( settings->value( location ).toString() );
                        if ( connection.baseUrl.isValid() && !connection.baseUrl.host().isEmpty() )
                        {
                                connections << connection;
                        }
                }
                settings->endGroup();

                return connections;
        }


        ///
        /// Convert an Odoo value to text
        ///
        QString OdooClient::valueToString( const QJsonValue& value, const QString& type )
        {
                if ( type == "boolean" )
                {
                        return value.toBool() ? "True" : "False";
                }

                switch ( value.type() )
                {
                case QJsonValue::Null:
                case QJsonValue::Undefined:
                        return QString();

                case QJsonValue::Bool:
                        // Odoo returns false for empty non-boolean fields
                        return value.toBool() ? "True" : QString();

                case QJsonValue::Double:
                        if ( type == "integer" || type == "many2one" )
                        {
                                return QString::number( value.toInteger() );
                        }
                        return QString::number( value.toDouble(), 'g', 15 );

                case QJsonValue::String:
                        return value.toString();

                case QJsonValue::Array:
                {
                        QJsonArray array = value.toArray();
                        // many2one: [id, "display name"]
                        if ( array.size() == 2 && array[0].isDouble() && array[1].isString() )
                        {
                                return array[1].toString();
                        }
                        QStringList parts;
                        for ( const auto& v : array )
                        {
                                parts << valueToString( v, QString() );
                        }
                        return parts.join( ',' );
                }

                case QJsonValue::Object:
                        return QString::fromUtf8( QJsonDocument( value.toObject() ).toJson( QJsonDocument::Compact ) );
                }

                return QString();
        }


        ///
        /// fields_get
        ///
        QList<OdooField> OdooClient::fieldsGet( const QString& model )
        {
                QList<OdooField> list;

                QJsonObject kwargs;
                kwargs["attributes"] = QJsonArray{ "string", "type", "store", "relation" };
                QJsonValue result = call( model, "fields_get", QJsonArray(), kwargs );
                if ( !result.isObject() )
                {
                        return list;
                }

                QJsonObject obj = result.toObject();
                for ( auto it = obj.begin(); it != obj.end(); ++it )
                {
                        QJsonObject attrs = it.value().toObject();
                        OdooField field;
                        field.name   = it.key();
                        field.label  = attrs["string"].toString();
                        field.type   = attrs["type"].toString();
                        field.relation = attrs["relation"].toString();
                        field.stored = attrs["store"].toBool( true );
                        list << field;
                }

                std::sort( list.begin(), list.end(),
                           []( const OdooField& a, const OdooField& b ) { return a.name < b.name; } );
                return list;
        }


        bool OdooField::isRelational() const
        {
                return !relation.isEmpty()
                        && ( type == "many2one" || type == "one2many" || type == "many2many" );
        }


        bool OdooField::isToMany() const
        {
                return isRelational() && type != "many2one";
        }


        ///
        /// read: records by id, archived ones included (unlike search_read)
        ///
        QJsonArray OdooClient::read( const QString& model, const QList<int>& ids, const QStringList& fields )
        {
                QJsonArray idArray;
                for ( int id : ids )
                {
                        idArray << id;
                }

                QJsonObject kwargs;
                kwargs["fields"] = QJsonArray::fromStringList( fields );
                return call( model, "read", idArray, kwargs ).toArray();
        }


        ///
        /// search_read
        ///
        QJsonArray OdooClient::searchRead( const QString& model,
                                           const QJsonArray& domain,
                                           const QStringList& fields,
                                           int limit,
                                           const QString& order )
        {
                QJsonObject kwargs;
                kwargs["domain"] = domain;
                kwargs["fields"] = QJsonArray::fromStringList( fields );
                if ( limit > 0 )      kwargs["limit"] = limit;
                if ( !order.isEmpty() ) kwargs["order"] = order;

                return call( model, "search_read", QJsonArray(), kwargs ).toArray();
        }


        QJsonObject OdooClient::context() const
        {
                QJsonObject ctx;
                if ( !mSource.lang.isEmpty() )
                {
                        ctx["lang"] = mSource.lang;
                }
                return ctx;
        }


        ///
        /// Call a model method, choosing the protocol on first use
        ///
        QJsonValue OdooClient::call( const QString& model,
                                     const QString& method,
                                     const QJsonArray& ids,
                                     const QJsonObject& kwargs )
        {
                mError.clear();

                if ( mApiKey.isEmpty() )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                              "No API key for %1" ).arg( mSource.keyLocation() );
                        return QJsonValue();
                }

                if ( mProtocol != Protocol::JsonRpc )
                {
                        QJsonObject params = kwargs;
                        params["context"] = context();
                        if ( !ids.isEmpty() )
                        {
                                params["ids"] = ids;
                        }

                        bool notFound = false;
                        QJsonValue result = callJson2( model, method, params, notFound );
                        if ( !notFound )
                        {
                                mProtocol = Protocol::Json2;
                                return result;
                        }
                        if ( mProtocol == Protocol::Json2 )
                        {
                                return result;
                        }
                        mError.clear();
                        mProtocol = Protocol::JsonRpc;
                }

                // Legacy /jsonrpc (Odoo <= 18)
                if ( mSource.database.isEmpty() )
                {
                        QJsonValue dbs = callJsonRpc( "db", "list", QJsonArray() );
                        if ( dbs.isArray() && dbs.toArray().size() == 1 )
                        {
                                mSource.database = dbs.toArray()[0].toString();
                        }
                        else
                        {
                                mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                                      "This server needs a database name (no JSON-2 API)" );
                                return QJsonValue();
                        }
                }

                if ( mUid == 0 )
                {
                        if ( mSource.login.isEmpty() )
                        {
                                mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                                      "This server needs a login (no JSON-2 API)" );
                                return QJsonValue();
                        }

                        QJsonValue uid = callJsonRpc( "common", "authenticate",
                                                      QJsonArray{ mSource.database, mSource.login, mApiKey, QJsonObject() } );
                        if ( !mError.isEmpty() )
                        {
                                return QJsonValue();
                        }
                        if ( !uid.isDouble() || uid.toInt() == 0 )
                        {
                                mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                                      "Authentication failed for %1" ).arg( mSource.login );
                                return QJsonValue();
                        }
                        mUid = uid.toInt();
                }

                QJsonObject legacyKwargs = kwargs;
                legacyKwargs["context"] = context();

                // Record methods take the ids as the first positional argument
                QJsonArray args;
                if ( !ids.isEmpty() )
                {
                        args.append( ids );
                }

                return callJsonRpc( "object", "execute_kw",
                                    QJsonArray{ mSource.database, mUid, mApiKey, model, method, args, legacyKwargs } );
        }


        ///
        /// create: returns the new record id, 0 on error
        ///
        int OdooClient::create( const QString& model, const QJsonObject& values )
        {
                QJsonObject kwargs;
                kwargs["vals_list"] = QJsonArray{ values };
                QJsonValue result = call( model, "create", QJsonArray(), kwargs );

                // JSON-2 and execute_kw both return the list of new ids
                if ( result.isArray() && !result.toArray().isEmpty() )
                {
                        return result.toArray()[0].toInt();
                }
                if ( result.isDouble() )
                {
                        return result.toInt();
                }
                if ( mError.isEmpty() )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                              "The server did not return the new record" );
                }
                return 0;
        }


        ///
        /// write
        ///
        bool OdooClient::write( const QString& model, const QList<int>& ids, const QJsonObject& values )
        {
                QJsonArray idArray;
                for ( int id : ids )
                {
                        idArray.append( id );
                }

                QJsonObject kwargs;
                kwargs["vals"] = values;
                call( model, "write", idArray, kwargs );
                return mError.isEmpty();
        }


        ///
        /// ir.model id of a model, 0 if unknown
        ///
        int OdooClient::modelId( const QString& model )
        {
                QJsonArray rows = searchRead( "ir.model",
                                              QJsonArray{ QJsonArray{ "model", "=", model } },
                                              { "id" }, 1, QString() );
                if ( rows.isEmpty() )
                {
                        return 0;
                }
                return rows[0].toObject()["id"].toInt();
        }


        ///
        /// JSON-2: POST /json/2/<model>/<method>
        ///
        QJsonValue OdooClient::callJson2( const QString& model,
                                          const QString& method,
                                          const QJsonObject& params,
                                          bool& notFound )
        {
                notFound = false;

                QUrl url = OdooSource::normalizedBaseUrl( mSource.baseUrl );
                url.setPath( url.path() + "/json/2/" + model + "/" + method );

                int status = 0;
                QByteArray reply;
                if ( !post( url, params, mApiKey, status, reply ) )
                {
                        return QJsonValue();
                }

                bool parsed = false;
                QJsonValue value = parseReply( reply, parsed );

                if ( status == 404 && !parsed )
                {
                        // HTML 404 page: the endpoint does not exist (Odoo <= 18)
                        notFound = true;
                        return QJsonValue();
                }

                if ( !parsed )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                              "Invalid reply from server (HTTP %1)" ).arg( status );
                        return QJsonValue();
                }

                if ( status != 200 )
                {
                        mError = errorFromJson( value.toObject() );
                        if ( mError.isEmpty() )
                        {
                                mError = QString( "HTTP %1" ).arg( status );
                        }
                        return QJsonValue();
                }

                return value;
        }


        ///
        /// Parse any JSON value, including the bare scalars JSON-2 returns
        /// (write/unlink answer "true"), which QJsonDocument alone rejects
        ///
        QJsonValue OdooClient::parseReply( const QByteArray& reply, bool& ok )
        {
                ok = false;
                if ( reply.trimmed().isEmpty() )
                {
                        return QJsonValue();
                }

                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson( "[" + reply + "]", &parseError );
                if ( parseError.error != QJsonParseError::NoError || doc.array().size() != 1 )
                {
                        return QJsonValue();
                }

                ok = true;
                return doc.array()[0];
        }


        ///
        /// Legacy JSON-RPC: POST /jsonrpc
        ///
        QJsonValue OdooClient::callJsonRpc( const QString& service,
                                            const QString& method,
                                            const QJsonArray& args )
        {
                QUrl url = OdooSource::normalizedBaseUrl( mSource.baseUrl );
                url.setPath( url.path() + "/jsonrpc" );

                QJsonObject body;
                body["jsonrpc"] = "2.0";
                body["method"]  = "call";
                body["id"]      = 1;
                body["params"]  = QJsonObject{ { "service", service }, { "method", method }, { "args", args } };

                int status = 0;
                QByteArray reply;
                if ( !post( url, body, QString(), status, reply ) )
                {
                        return QJsonValue();
                }

                QJsonParseError parseError;
                QJsonObject obj = QJsonDocument::fromJson( reply, &parseError ).object();
                if ( parseError.error != QJsonParseError::NoError )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                              "Invalid reply from server (HTTP %1)" ).arg( status );
                        return QJsonValue();
                }

                if ( obj.contains( "error" ) )
                {
                        mError = errorFromJson( obj );
                        return QJsonValue();
                }

                return obj["result"];
        }


        ///
        /// Synchronous POST with timeout
        ///
        bool OdooClient::post( const QUrl& url,
                               const QJsonObject& body,
                               const QString& bearer,
                               int& status,
                               QByteArray& reply )
        {
                QNetworkAccessManager manager;

                QNetworkRequest request( url );
                request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8" );
                request.setHeader( QNetworkRequest::UserAgentHeader, "gLabels-qt Odoo merge" );
                if ( !bearer.isEmpty() )
                {
                        request.setRawHeader( "Authorization", "bearer " + bearer.toUtf8() );
                }
                if ( !mSource.database.isEmpty() )
                {
                        request.setRawHeader( "X-Odoo-Database", mSource.database.toUtf8() );
                }
                request.setAttribute( QNetworkRequest::RedirectPolicyAttribute,
                                      QNetworkRequest::NoLessSafeRedirectPolicy );

                QNetworkReply* networkReply = manager.post( request, QJsonDocument( body ).toJson( QJsonDocument::Compact ) );

                QEventLoop loop;
                QTimer timer;
                timer.setSingleShot( true );
                QObject::connect( &timer, &QTimer::timeout, networkReply, &QNetworkReply::abort );
                QObject::connect( networkReply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
                timer.start( timeoutMs );
                loop.exec( QEventLoop::ExcludeUserInputEvents );

                bool timedOut = !timer.isActive();
                status = networkReply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
                reply  = networkReply->readAll();

                bool ok = true;
                if ( timedOut )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                              "Timeout connecting to %1" ).arg( url.host() );
                        ok = false;
                }
                else if ( status == 0 )
                {
                        // No HTTP response at all (DNS, TLS, connection refused...)
                        mError = networkReply->errorString();
                        ok = false;
                }

                // The reply belongs to the local manager; free it before the manager goes away
                delete networkReply;
                return ok;
        }


        ///
        /// Split the columns text
        ///
        QStringList OdooReport::parseColumns( const QString& text )
        {
                QStringList columns;
                for ( const auto& part : text.split( ',', Qt::SkipEmptyParts ) )
                {
                        QString column = part.trimmed();
                        if ( !column.isEmpty() && !columns.contains( column ) )
                        {
                                columns << column;
                        }
                }
                return columns;
        }


        ///
        /// Columns gLabels cannot use as field names
        ///
        QStringList OdooReport::invalidColumns( const QStringList& columns )
        {
                QStringList bad;
                for ( const auto& column : columns )
                {
                        for ( const auto& ch : FORBIDDEN_CHARS )
                        {
                                if ( column.contains( ch ) )
                                {
                                        bad << column;
                                        break;
                                }
                        }
                }
                return bad;
        }


        namespace
        {
                QString csvCell( QString cell )
                {
                        if ( cell.contains( ',' ) || cell.contains( '"' ) || cell.contains( '\n' ) || cell.contains( '\r' ) )
                        {
                                cell.replace( "\"", "\"\"" );
                                cell = "\"" + cell + "\"";
                        }
                        return cell;
                }
        }


        ///
        /// First CSV line: the column contract with report_glabels
        ///
        QByteArray OdooReport::csvHeader( const QStringList& columns )
        {
                return csvFile( columns, {} );
        }


        ///
        /// Whole CSV: the columns, then sample rows for the next designer
        /// (report_glabels reads only the first line)
        ///
        QByteArray OdooReport::csvFile( const QStringList& columns, const QList<QStringList>& rows )
        {
                QByteArray csv;
                for ( const QStringList& line : QList<QStringList>{ columns } + rows )
                {
                        QStringList cells;
                        for ( const auto& cell : line )
                        {
                                cells << csvCell( cell );
                        }
                        csv += cells.join( ',' ).toUtf8() + "\n";
                }
                return csv;
        }


        ///
        /// All CSV rows (BOM, quotes, quoted line breaks and CRLF allowed)
        ///
        QList<QStringList> OdooReport::parseCsv( const QByteArray& csv )
        {
                QString text = QString::fromUtf8( csv );
                if ( text.startsWith( QChar( 0xFEFF ) ) )
                {
                        text.remove( 0, 1 );
                }

                QList<QStringList> rows;
                QStringList row;
                QString cell;
                bool quoted = false;
                bool pending = false;   // something read since the last line end
                for ( int i = 0; i < text.size(); i++ )
                {
                        QChar ch = text[i];
                        if ( quoted )
                        {
                                if ( ch == '"' && i + 1 < text.size() && text[i+1] == '"' )
                                {
                                        cell += '"';
                                        i++;
                                }
                                else if ( ch == '"' )
                                {
                                        quoted = false;
                                }
                                else
                                {
                                        cell += ch;
                                }
                        }
                        else if ( ch == '"' )
                        {
                                quoted = true;
                                pending = true;
                        }
                        else if ( ch == ',' )
                        {
                                row << cell;
                                cell.clear();
                                pending = true;
                        }
                        else if ( ch == '\n' || ch == '\r' )
                        {
                                if ( ch == '\r' && i + 1 < text.size() && text[i+1] == '\n' )
                                {
                                        i++;
                                }
                                if ( pending || !cell.isEmpty() )
                                {
                                        row << cell;
                                        rows << row;
                                }
                                row.clear();
                                cell.clear();
                                pending = false;
                        }
                        else
                        {
                                cell += ch;
                                pending = true;
                        }
                }
                if ( pending || !cell.isEmpty() )
                {
                        row << cell;
                        rows << row;
                }
                return rows;
        }


        ///
        /// Columns from the first line of a report CSV
        ///
        QStringList OdooReport::parseCsvHeader( const QByteArray& csv )
        {
                QStringList columns;
                for ( const auto& cell : parseCsv( csv ).value( 0 ) )
                {
                        if ( !cell.trimmed().isEmpty() )
                        {
                                columns << cell.trimmed();
                        }
                }
                return columns;
        }


        const QStringList& OdooReport::magicColumns()
        {
                static const QStringList columns = { "creator", "company", "date_now", "currency" };
                return columns;
        }


        ///
        /// Technical report name: glabels_<model>_<name>, ASCII only
        ///
        QString OdooReport::reportNameStem( const QString& model, const QString& name )
        {
                static const QRegularExpression notAllowed( "[^a-z0-9]+" );

                QString modelPart = model.toLower();
                modelPart.replace( notAllowed, "_" );

                QString namePart = name.normalized( QString::NormalizationForm_KD ).toLower();
                namePart.replace( notAllowed, "_" );
                namePart = namePart.section( '_', 0, -1, QString::SectionSkipEmpty );

                QString stem = "glabels_" + modelPart;
                if ( !namePart.isEmpty() )
                {
                        stem += "_" + namePart;
                }
                return stem.left( 100 );
        }

}
