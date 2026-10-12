//  Merge/OdooClient.hpp
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

#ifndef merge_OdooClient_hpp
#define merge_OdooClient_hpp


#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>


namespace glabels::merge
{

        ///
        /// Odoo merge source description
        ///
        /// Serialized as a URL, e.g.
        ///   https://erp.example.com/?db=prod&model=stock.lot&fields=name,product_id&limit=80
        ///
        /// The API key is never part of the source, so a template uploaded to
        /// Odoo does not carry credentials.  It is looked up with apiKey().
        ///
        struct OdooSource
        {
                QUrl        baseUrl;      // scheme://host[:port][/path]
                QString     database;     // optional for JSON-2, required for /jsonrpc
                QString     login;        // only needed for the /jsonrpc fallback
                QString     model;        // e.g. "stock.lot"
                QStringList fields;       // empty = all simple fields
                QString     domain{ "[]" }; // JSON domain, e.g. [["product_id.default_code","=","PR6030"]]
                QString     order;        // e.g. "name asc"
                QString     lang;         // e.g. "bg_BG", empty = user language
                int         limit{ 80 };

                bool isValid() const;

                QString toString() const;
                static OdooSource fromString( const QString& source );

                QString keyLocation() const;

                static QUrl normalizedBaseUrl( const QUrl& url );
        };


        ///
        /// Field description returned by fields_get
        ///
        struct OdooField
        {
                QString name;
                QString label;
                QString type;
                bool    stored{ true };
        };


        ///
        /// Minimal synchronous Odoo RPC client
        ///
        /// Tries the JSON-2 API first (Odoo 19+: /json/2/<model>/<method> with a
        /// bearer API key) and falls back to the legacy /jsonrpc execute_kw
        /// endpoint (Odoo <= 18), which additionally needs database and login.
        ///
        class OdooClient
        {
        public:
                OdooClient( const OdooSource& source, const QString& apiKey );

                QList<OdooField> fieldsGet( const QString& model );
                QJsonArray searchRead( const QString& model,
                                       const QJsonArray& domain,
                                       const QStringList& fields,
                                       int limit,
                                       const QString& order );

                QString errorString() const;

                // API key storage (outside of the label file)
                static QString apiKey( const OdooSource& source );
                static void saveApiKey( const OdooSource& source, const QString& apiKey, bool persist = true );

                // Convert an Odoo field value into merge text
                static QString valueToString( const QJsonValue& value, const QString& type );

                static int timeoutMs;

        private:
                QJsonValue call( const QString& model,
                                 const QString& method,
                                 const QJsonArray& args,
                                 const QJsonObject& kwargs );
                QJsonValue callJson2( const QString& model,
                                      const QString& method,
                                      const QJsonObject& params,
                                      bool& notFound );
                QJsonValue callJsonRpc( const QString& service,
                                        const QString& method,
                                        const QJsonArray& args );
                QJsonObject context() const;

                bool post( const QUrl& url,
                           const QJsonObject& body,
                           const QString& bearer,
                           int& status,
                           QByteArray& reply );

                enum class Protocol { Unknown, Json2, JsonRpc };

                OdooSource mSource;
                QString    mApiKey;
                Protocol   mProtocol{ Protocol::Unknown };
                int        mUid{ 0 };
                QString    mError;
        };

}


#endif // merge_OdooClient_hpp
