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

#include <QJsonArray>


namespace glabels::merge
{

        ///
        /// Odoo RPC Merge Backend
        ///
        /// Source is an OdooSource URL; keys are the model's fields (from
        /// fields_get), records come from search_read.
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
                QMap<QString,QString>  mTypes;
                QJsonArray             mRows;
                int                    mIndex{ 0 };
                QString                mError;
        };

}


#endif // merge_OdooRpc_hpp
