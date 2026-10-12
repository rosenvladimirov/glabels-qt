//  Merge/OdooRpc.hpp
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

#ifndef merge_OdooRpc_hpp
#define merge_OdooRpc_hpp


#include "Merge.hpp"
#include "OdooClient.hpp"
#include "Record.hpp"

#include <QJsonArray>
#include <QMap>


namespace glabels::merge
{

        ///
        /// Odoo RPC Merge Backend
        ///
        /// Source is an OdooSource URL; records come from search_read.  The
        /// keys are the report columns of report_glabels: fields of the model,
        /// field paths through related models (product_tmpl_id.name,
        /// product_variant_ids.default_code), report_glabels values (company,
        /// date_now...) and columns of other modules (price).  What cannot be
        /// read from Odoo is shown with a fallback value, e.g. the sample row
        /// of the report CSV.
        ///
        struct OdooRpc : public Merge
        {

                /////////////////////////////////
                // Life Cycle
                /////////////////////////////////
        private:
                OdooRpc();
                OdooRpc( const OdooRpc* merge );
                virtual ~OdooRpc() = default;


                /////////////////////////////////
                // Object duplication
                /////////////////////////////////
        public:
                OdooRpc* clone() const override;


                /////////////////////////////////
                // Static methods
                /////////////////////////////////
        public:
                static QString id();
                static Merge* create();

                static bool parseDomain( const QString& text, QJsonArray& domain );


                /////////////////////////////////
                // Report columns
                /////////////////////////////////
        public:
                // Values for columns Odoo cannot fill here (set before setSource)
                void setFallbackValues( const QMap<QString,QString>& values );

                // Columns that are neither fields nor field paths of the model
                QStringList unresolvedKeys() const;

                // Why the report engine could not fill the columns (old module, missing pricelist...)
                QString previewError() const;

                // First record as a CSV sample row; unresolved columns keep the fallback
                QStringList sampleRow( const QStringList& columns ) const;


                /////////////////////////////////
                // Implementation of virtual methods
                /////////////////////////////////
        public:
                QStringList keys() const override;
                QString primaryKey() const override;
                QString errorString() const override;
        protected:
                void open() override;
                void close() override;
                Record readNextRecord() override;


                /////////////////////////////////
                // Private data
                /////////////////////////////////
        private:
                QStringList            mKeys;
                QStringList            mUnresolved;
                QMap<QString,QString>  mFallback;
                QList<Record>          mRecords;
                int                    mIndex{ 0 };
                QString                mError;
                QString                mPreviewError;
        };

}


#endif // merge_OdooRpc_hpp
