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
                settings->beginGroup( "keys" );
                QString encoded = QString::fromUtf8( QUrl::toPercentEncoding( source.keyLocation() ) );
                if ( apiKey.isEmpty() )
                {
                        settings->remove( encoded );
                }
                else
                {
                        settings->setValue( encoded, apiKey );
                }
                settings->endGroup();
                settings->sync();

                QFile::setPermissions( settings->fileName(), QFile::ReadOwner | QFile::WriteOwner );
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
                kwargs["attributes"] = QJsonArray{ "string", "type", "store" };
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
                        field.stored = attrs["store"].toBool( true );
                        list << field;
                }

                std::sort( list.begin(), list.end(),
                           []( const OdooField& a, const OdooField& b ) { return a.name < b.name; } );
                return list;
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
                                     const QJsonArray& args,
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

                return callJsonRpc( "object", "execute_kw",
                                    QJsonArray{ mSource.database, mUid, mApiKey, model, method, args, legacyKwargs } );
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

                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson( reply, &parseError );

                if ( status == 404 && parseError.error != QJsonParseError::NoError )
                {
                        // HTML 404 page: the endpoint does not exist (Odoo <= 18)
                        notFound = true;
                        return QJsonValue();
                }

                if ( parseError.error != QJsonParseError::NoError )
                {
                        mError = QCoreApplication::translate( "glabels::merge::OdooClient",
                                                              "Invalid reply from server (HTTP %1)" ).arg( status );
                        return QJsonValue();
                }

                if ( status != 200 )
                {
                        mError = errorFromJson( doc.object() );
                        if ( mError.isEmpty() )
                        {
                                mError = QString( "HTTP %1" ).arg( status );
                        }
                        return QJsonValue();
                }

                if ( doc.isArray() )
                {
                        return doc.array();
                }
                return doc.object();
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

}
