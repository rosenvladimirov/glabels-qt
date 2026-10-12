//  TestMerge.cpp
//
//  Copyright (C) 2019-2026  Jaye Evins <evins@snaught.com>
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


#include "TestMerge.hpp"

#include "merge/Factory.hpp"
#include "merge/None.hpp"
#include "merge/TextCsv.hpp"
#include "merge/TextCsvKeys.hpp"
#include "merge/TextTsv.hpp"
#include "merge/TextTsvKeys.hpp"
#include "merge/TextColon.hpp"
#include "merge/TextColonKeys.hpp"
#include "merge/TextSemicolon.hpp"
#include "merge/TextSemicolonKeys.hpp"

#include "merge/Record.hpp"

#if GLABELS_ODOO_MERGE
#include "merge/OdooClient.hpp"
#include "merge/OdooRpc.hpp"
#endif

#include <QDebug>


QTEST_MAIN(TestMerge)

Q_DECLARE_METATYPE(glabels::merge::Factory::SourceType)

using namespace glabels::merge;


void TestMerge::initTestCase()
{
        Factory::init();
}


void TestMerge::factory_data()
{
        QTest::addColumn<QString>( "id" );
        QTest::addColumn<QString>( "name" );
        QTest::addColumn<Factory::SourceType>( "type" );
        QTest::addColumn<int>( "index" );

        int index = 0;
        QTest::newRow( "None" ) << None::id() << "None" << Factory::NONE << index++;
        QTest::newRow( "TextCsv" ) << TextCsv::id() << "Text: Comma Separated Values (CSV)" << Factory::FILE << index++;
        QTest::newRow( "TextCsvKeys" ) << TextCsvKeys::id() << "Text: Comma Separated Values (CSV), keys on line 1" << Factory::FILE << index++;
        QTest::newRow( "TextTsv" ) << TextTsv::id() << "Text: Tab Separated Values (TSV)" << Factory::FILE << index++;
        QTest::newRow( "TextTsvKeys" ) << TextTsvKeys::id() << "Text: Tab Separated Values (TSV), keys on line 1" << Factory::FILE << index++;
        QTest::newRow( "TextColon" ) << TextColon::id() << "Text: Colon Separated Values" << Factory::FILE << index++;
        QTest::newRow( "TextColonKeys" ) << TextColonKeys::id() << "Text: Colon Separated Values, keys on line 1" << Factory::FILE << index++;
        QTest::newRow( "TextSemicolon" ) << TextSemicolon::id() << "Text: Semicolon Separated Values" << Factory::FILE << index++;
        QTest::newRow( "TextSemicolonKeys" ) << TextSemicolonKeys::id() << "Text: Semicolon Separated Values, keys on line 1" << Factory::FILE << index++;
#if GLABELS_ODOO_MERGE
        QTest::newRow( "OdooRpc" ) << OdooRpc::id() << "Odoo: records via RPC (API key)" << Factory::REMOTE << index++;
#endif
}


void TestMerge::factory()
{
        QFETCH( QString, id );
        QFETCH( QString, name );
        QFETCH( Factory::SourceType, type );
        QFETCH( int, index );

        QVERIFY( Factory::nameList().contains( name ) );

        QString outName = Factory::idToName( id );
        QCOMPARE( outName, name );

        QString outId = Factory::nameToId( name );
        QCOMPARE( outId, id );

        Factory::SourceType outType = Factory::idToType( id );
        QCOMPARE( outType, type );

        outId = Factory::indexToId( index );
        QCOMPARE( outId, id );

        Merge* merge = Factory::createMerge( id );
        QVERIFY( merge );
        QCOMPARE( merge->id(), id );

        Merge* cloneMerge = merge->clone();
        QCOMPARE( cloneMerge->id(), merge->id() );
        delete cloneMerge;
        delete merge;
}


void TestMerge::factoryNotRegistered()
{
        QString id( "unregistered" );
        Merge* merge = Factory::createMerge( id );
        QVERIFY( merge );
        QVERIFY( merge->id() != id );
        QCOMPARE( merge->id(), None::id() );
        delete merge;
}


void TestMerge::text_data()
{
        QTest::addColumn<QString>( "id" );
        QTest::addColumn<bool>( "keyed" );
        QTest::addColumn<char>( "delim" );

        QTest::newRow( "TextCsv" ) << TextCsv::id() << false << ',';
        QTest::newRow( "TextCsvKeys" ) << TextCsvKeys::id() << true << ',';
        QTest::newRow( "TextTsv" ) << TextTsv::id() << false << '\t';
        QTest::newRow( "TextTsvKeys" ) << TextTsvKeys::id() << true << '\t';
        QTest::newRow( "TextColon" ) << TextColon::id() << false << ':';
        QTest::newRow( "TextColonKeys" ) << TextColonKeys::id() << true << ':';
        QTest::newRow( "TextSemicolon" ) << TextSemicolon::id() << false << ';';
        QTest::newRow( "TextSemicolonKeys" ) << TextSemicolonKeys::id() << true << ';';
}


void TestMerge::text()
{
        QFETCH( QString, id );
        QFETCH( bool, keyed );
        QFETCH( char, delim );

        QTemporaryFile file;
        file.open();
        if ( keyed )
        {
                file.write( "header1" );
                file.putChar( delim );
                file.write( "\"header 2\"" );
                file.putChar( delim );
                file.write( "header3\r\n" );
        }
        file.write( "  val11" ); // Leading spaces in SIMPLE entry
        file.putChar( delim );
        file.write( "\"\"\"val 12\"\"\"" ); // 2DQUOTE at beginning and end of DQUOTE entry
        file.putChar( delim );
        file.write( "  \"val 13\"\n" ); // Leading spaces before DQUOTE entry, end line with LF only

        file.write( "\"  val21\"\"\"" ); // Leading spaces within DQUOTE entry, 2DQUOTE at end
        file.putChar( delim );
        file.write( "\"\"\"val 22\"" ); // 2DQUOTE at beginning of DQUOTE entry
        file.putChar( delim );
        file.write( "\r\n" ); // Last field blank

        file.write( "\"\"\"\"\"\"" ); // 2 2DQUOTES alone in DQUOTE entry
        file.putChar( delim );
        file.write( "val \"32" ); // DQUOTE in SIMPLE entry
        file.putChar( delim );
        file.write( "val \"\\\"33\r\n" ); // DQUOTE backslashed-DQUOTE in SIMPLE entry

        file.putChar( delim ); file.putChar( delim ); // All fields blank
        file.write( "\r\n" );

        file.write( "val\\n \\t \\r \\\\ \\x51" ); // Backslashed-n/-t/-r/-backslash/-x in SIMPLE entry
        file.putChar( delim );
        file.write( "\"val\\n \\t \\r \\\\ \\x52\"" ); // Backslashed-n/-t/-r/-backslash/-x in QUOTE entry
        file.write( "\r\n" ); // No last delim

        file.write( "\"val \"\"61\"" ); // 2DQUOTE in middle of DQUOTE entry
        file.putChar( delim );
        file.write( "\"val\"\"" ); file.putChar( delim ); file.write( "\r\n\\\"\u2019\\\\52\"" ); // 2DQUOTE delim CRLF backslashed-DQUOTE U+2019 backslashed-backslash
        file.putChar( delim );
        file.write( "\"val63\"" ); // End without CRLF
        file.close();

        Merge* merge = Factory::createMerge( id );
        QCOMPARE( merge->id(), id );

        merge->setSource( file.fileName() );
        QCOMPARE( merge->source(), file.fileName() );

        const QList<Record>& recordList = merge->recordList();
        QCOMPARE( recordList.size(), 6 );

        //
        // Records
        //
        const char* h1 = keyed ? "header1" : "1";
        const char* h2 = keyed ? "header 2" : "2";
        const char* h3 = keyed ? "header3" : "3";
        Record record;

        record = recordList[0];
        QVERIFY( record.contains( h1 ) );
        QCOMPARE( record.value( h1 ), QString( "  val11" ) );
        QVERIFY( record.contains( h2 ) );
        QCOMPARE( record.value( h2 ), QString( "\"val 12\"" ) );
        QVERIFY( record.contains( h3 ) );
        QCOMPARE( record.value( h3 ), QString( "  \"val 13\"" ) ); // NOTE: Treats as unquoted due to leading spaces

        record = recordList[1];
        QVERIFY( record.contains( h1 ) );
        QCOMPARE( record.value( h1 ), QString( "  val21\"" ) );
        QVERIFY( record.contains( h2 ) );
        QCOMPARE( record.value( h2 ), QString( "\"val 22" ) );
        QVERIFY( record.contains( h3 ) );
        QCOMPARE( record.value( h3 ), QString( "" ) );

        record = recordList[2];
        QVERIFY( record.contains( h1 ) );
        QCOMPARE( record.value( h1 ), QString( "\"\"" ) );
        QVERIFY( record.contains( h2 ) );
        QCOMPARE( record.value( h2 ), QString( "val \"32" ) );
        QVERIFY( record.contains( h3 ) );
        QCOMPARE( record.value( h3 ), QString( "val \"\"33" ) );

        record = recordList[3];
        QVERIFY( record.contains( h1 ) );
        QCOMPARE( record.value( h1 ), QString( "" ) );
        QVERIFY( record.contains( h2 ) );
        QCOMPARE( record.value( h2 ), QString( "" ) );
        QVERIFY( record.contains( h3 ) );
        QCOMPARE( record.value( h3 ), QString( "" ) );

        record = recordList[4];
        QVERIFY( record.contains( h1 ) );
        QCOMPARE( record.value( h1 ), QString( "val\n \t r \\ x51" ) );
        QVERIFY( record.contains( h2 ) );
        QCOMPARE( record.value( h2 ), QString( "val\n \t r \\ x52" ) );
        QVERIFY( !record.contains( h3 ) );

        record = recordList[5];
        QVERIFY( record.contains( h1 ) );
        QCOMPARE( record.value( h1 ), QString( "val \"61" ) );
        QVERIFY( record.contains( h2 ) );
        QCOMPARE( record.value( h2 ), QString( "val\"" ).append( delim ).append( "\n\"\u2019\\52" ) ); // NOTE: CR missing (QIODevice::Text strips all CRs from stream)
        QVERIFY( record.contains( h3 ) );
        QCOMPARE( record.value( h3 ), QString( "val63" ) );

        //
        // Selection
        //
        QCOMPARE( merge->nSelectedRecords(), 6 ); // Initially all selected
        merge->unselectAll();
        QCOMPARE( merge->nSelectedRecords(), 0 );

        merge->setSelected( 1 );
        QCOMPARE( merge->nSelectedRecords(), 1 );
        QCOMPARE( merge->selectedRecords().size(), 1 );

        merge->setSelected( 1, false );
        QCOMPARE( merge->nSelectedRecords(), 0 );
        QCOMPARE( merge->selectedRecords().size(), 0 );

        merge->setSelected( 0 );
        merge->setSelected( 3 );
        QCOMPARE( merge->nSelectedRecords(), 2 );
        QCOMPARE( merge->selectedRecords().size(), 2 );
        QCOMPARE( merge->selectedRecords().first(), recordList[0] );
        QCOMPARE( merge->selectedRecords().last(), recordList[3] );

        merge->setSelected( 0, false );
        QCOMPARE( merge->nSelectedRecords(), 1 );
        QCOMPARE( merge->selectedRecords().size(), 1 );

        //
        // Keys
        //
        QStringList keys = merge->keys();
        QCOMPARE( keys.size(), 3 );
        QCOMPARE( keys[0], QString( h1 ) );
        QCOMPARE( keys[1], QString( h2 ) );
        QCOMPARE( keys[2], QString( h3 ) );
        QCOMPARE( merge->primaryKey(), QString( h1 ) );

        //
        // Clone
        //
        merge->unselectAll();
        merge->setSelected( 0 );
        QCOMPARE( merge->nSelectedRecords(), 1 );

        Merge* cloneMerge = merge->clone();
        QCOMPARE( cloneMerge->id(), merge->id() );
        QCOMPARE( cloneMerge->source(), merge->source() );
        QCOMPARE( cloneMerge->recordList().size(), merge->recordList().size() );
        QCOMPARE( cloneMerge->recordList()[0], merge->recordList()[0] );
        QCOMPARE( cloneMerge->recordList()[1], merge->recordList()[1] );
        QCOMPARE( cloneMerge->recordList()[2], merge->recordList()[2] );
        QCOMPARE( cloneMerge->recordList()[3], merge->recordList()[3] );
        QCOMPARE( cloneMerge->recordList()[4], merge->recordList()[4] );
        QCOMPARE( cloneMerge->recordList()[5], merge->recordList()[5] );
        QCOMPARE( cloneMerge->nSelectedRecords(), merge->nSelectedRecords() );
        QCOMPARE( cloneMerge->selectedRecords().size(), merge->selectedRecords().size() );
        QCOMPARE( cloneMerge->selectedRecords()[0], merge->selectedRecords()[0] );
        QCOMPARE( cloneMerge->keys(), merge->keys() );
        QCOMPARE( cloneMerge->primaryKey(), merge->primaryKey() );
        delete cloneMerge;
        delete merge;
}


void TestMerge::none()
{
        None none;
        QCOMPARE( none.id(), QString( "None" ) );

        None* cloneNone = none.clone();
        QCOMPARE( cloneNone->id(), none.id() );
        QCOMPARE( cloneNone->keys(), none.keys() );
        QCOMPARE( cloneNone->primaryKey(), none.primaryKey() );
        delete cloneNone;
}


void TestMerge::record()
{
        Record record;
        QCOMPARE( record.isSelected(), true );
        record.setSelected( false );
        QCOMPARE( record.isSelected(), false );
        record.setSelected( true );
        QCOMPARE( record.isSelected(), true );

        record["key"] = "val";
        QVERIFY( record.contains( "key" ) );
        QCOMPARE( record["key"], QString( "val" ) );

        Record cloneRecord = record;
        QCOMPARE( cloneRecord.isSelected(), true );
        QVERIFY( cloneRecord.contains( "key" ) );
        QCOMPARE( cloneRecord.value( "key" ), QString( "val" ) );

        record.setSelected( false );
        Record record2( record );
        QCOMPARE( record2.isSelected(), false );
        QVERIFY( record2.contains( "key" ) );
        QCOMPARE( record2["key"], QString( "val" ) );
}


void TestMerge::odooSource()
{
#if GLABELS_ODOO_MERGE
        OdooSource src;
        src.baseUrl  = QUrl( "https://erp.example.com:8069" );
        src.database = "prod";
        src.login    = "admin";
        src.model    = "stock.lot";
        src.fields   = QStringList{ "name", "product_id" };
        src.domain   = R"([["name","=","A&B=C+D"],["product_id.default_code","ilike","PR 6030"]])";
        src.order    = "name desc, id";
        src.lang     = "bg_BG";
        src.limit    = 25;

        QString text = src.toString();
        QVERIFY( !text.contains( "admin@" ) );

        OdooSource back = OdooSource::fromString( text );
        QCOMPARE( back.baseUrl.toString(), QString( "https://erp.example.com:8069" ) );
        QCOMPARE( back.database, src.database );
        QCOMPARE( back.login, src.login );
        QCOMPARE( back.model, src.model );
        QCOMPARE( back.fields, src.fields );
        QCOMPARE( back.domain, src.domain );
        QCOMPARE( back.order, src.order );
        QCOMPARE( back.lang, src.lang );
        QCOMPARE( back.limit, src.limit );
        QVERIFY( back.isValid() );
        QCOMPARE( back.keyLocation(), QString( "https://erp.example.com:8069#prod" ) );

        // Defaults
        OdooSource minimal = OdooSource::fromString( "https://erp.example.com/?model=res.partner" );
        QVERIFY( minimal.isValid() );
        QCOMPARE( minimal.domain, QString( "[]" ) );
        QCOMPARE( minimal.limit, 80 );
        QVERIFY( minimal.fields.isEmpty() );

        QVERIFY( !OdooSource::fromString( "https://erp.example.com/" ).isValid() );

        // Same server with or without trailing slash -> same stored key
        OdooSource a = OdooSource::fromString( "https://erp.example.com/?model=res.partner" );
        OdooSource b;
        b.baseUrl = OdooSource::normalizedBaseUrl( QUrl( "https://erp.example.com" ) );
        QCOMPARE( a.keyLocation(), b.keyLocation() );

        // Sub-path installs keep their path
        OdooSource sub = OdooSource::fromString( "https://host/odoo/?model=res.partner" );
        QCOMPARE( sub.baseUrl.toString(), QString( "https://host/odoo" ) );
#else
        QSKIP( "Built without Qt6::Network" );
#endif
}


void TestMerge::odooDomain()
{
#if GLABELS_ODOO_MERGE
        QJsonArray domain;
        QVERIFY( OdooRpc::parseDomain( "[]", domain ) );
        QCOMPARE( domain.size(), 0 );

        QVERIFY( OdooRpc::parseDomain( R"([["active","=",true]])", domain ) );
        QCOMPARE( domain[0].toArray()[2].toBool(), true );

        // Python literal as copied from Odoo
        QVERIFY( OdooRpc::parseDomain( "[('active', '=', True), ('qty', '>', 0)]", domain ) );
        QCOMPARE( domain.size(), 2 );
        QCOMPARE( domain[0].toArray()[0].toString(), QString( "active" ) );
        QCOMPARE( domain[0].toArray()[2].toBool(), true );

        QVERIFY( !OdooRpc::parseDomain( "{\"a\": 1}", domain ) );
        QVERIFY( !OdooRpc::parseDomain( "[[", domain ) );
#else
        QSKIP( "Built without Qt6::Network" );
#endif
}


void TestMerge::odooValue()
{
#if GLABELS_ODOO_MERGE
        // many2one -> display name
        QCOMPARE( OdooClient::valueToString( QJsonArray{ 7, "MEC Product" }, "many2one" ), QString( "MEC Product" ) );
        // empty many2one / char -> empty text
        QCOMPARE( OdooClient::valueToString( false, "many2one" ), QString() );
        QCOMPARE( OdooClient::valueToString( false, "char" ), QString() );
        // boolean keeps False
        QCOMPARE( OdooClient::valueToString( false, "boolean" ), QString( "False" ) );
        QCOMPARE( OdooClient::valueToString( true, "boolean" ), QString( "True" ) );
        // numbers
        QCOMPARE( OdooClient::valueToString( 42, "integer" ), QString( "42" ) );
        QCOMPARE( OdooClient::valueToString( 12.5, "float" ), QString( "12.5" ) );
        // x2many ids
        QCOMPARE( OdooClient::valueToString( QJsonArray{ 1, 2, 3 }, "many2many" ), QString( "1,2,3" ) );
        QCOMPARE( OdooClient::valueToString( "Кирилица", "char" ), QString( "Кирилица" ) );
#else
        QSKIP( "Built without Qt6::Network" );
#endif
}


void TestMerge::odooReport()
{
#if GLABELS_ODOO_MERGE
        // Columns: trimmed, empty and duplicate parts dropped
        QStringList columns = OdooReport::parseColumns( " id, name ,,categ_id.name, name , date_now" );
        QCOMPARE( columns, ( QStringList{ "id", "name", "categ_id.name", "date_now" } ) );

        // ':' and '}' cannot be gLabels field names
        QCOMPARE( OdooReport::invalidColumns( { "name", "price:2", "a}b" } ), ( QStringList{ "price:2", "a}b" } ) );
        QVERIFY( OdooReport::invalidColumns( columns ).isEmpty() );

        // CSV header line, quoted only when needed
        QCOMPARE( OdooReport::csvHeader( { "id", "categ_id.name" } ), QByteArray( "id,categ_id.name\n" ) );
        QCOMPARE( OdooReport::csvHeader( { "a,b", "say \"hi\"" } ), QByteArray( "\"a,b\",\"say \"\"hi\"\"\"\n" ) );

        // ...and back: only the first line counts, BOM and CRLF are fine
        QCOMPARE( OdooReport::parseCsvHeader( OdooReport::csvHeader( { "id", "a,b", "say \"hi\"" } ) ),
                  ( QStringList{ "id", "a,b", "say \"hi\"" } ) );
        QCOMPARE( OdooReport::parseCsvHeader( "\xEF\xBB\xBFid, name\r\n1,Box\r\n" ), ( QStringList{ "id", "name" } ) );
        QVERIFY( OdooReport::parseCsvHeader( "" ).isEmpty() );

        // Technical report name
        QCOMPARE( OdooReport::reportNameStem( "product.product", "MEC Product label 60x30" ),
                  QString( "glabels_product_product_mec_product_label_60x30" ) );
        QCOMPARE( OdooReport::reportNameStem( "stock.lot", "Етикет партида" ), QString( "glabels_stock_lot" ) );
        QCOMPARE( OdooReport::reportNameStem( "stock.lot", "  --Lot--  " ), QString( "glabels_stock_lot_lot" ) );

        // JSON-2 replies: bare scalars (write -> true) are valid replies
        bool ok = false;
        QCOMPARE( OdooClient::parseReply( "true", ok ).toBool(), true );
        QVERIFY( ok );
        QCOMPARE( OdooClient::parseReply( "[1384]", ok ).toArray()[0].toInt(), 1384 );
        QVERIFY( ok );
        QCOMPARE( OdooClient::parseReply( "{\"name\": \"x\"}", ok ).toObject()["name"].toString(), QString( "x" ) );
        QVERIFY( ok );
        OdooClient::parseReply( "<html>404</html>", ok );
        QVERIFY( !ok );
        OdooClient::parseReply( "", ok );
        QVERIFY( !ok );
        OdooClient::parseReply( "1, 2", ok );
        QVERIFY( !ok );
#else
        QSKIP( "Built without Qt6::Network" );
#endif
}
