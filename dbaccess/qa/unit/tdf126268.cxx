/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dbtest_base.cxx"

#include <hsqlimport.hxx>

#include <osl/process.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <test/unoapi_test.hxx>
#include <svtools/miscopt.hxx>

using namespace dbahsql;

class Tdf126268Test : public DBTestBase
{
public:
    void testNumbers();

    virtual void setUp() override;

    CPPUNIT_TEST_SUITE(Tdf126268Test);

    CPPUNIT_TEST(testNumbers);

    CPPUNIT_TEST_SUITE_END();
};

void Tdf126268Test::setUp()
{
    DBTestBase::setUp();
    SvtMiscOptions aMiscOptions;
    osl_setEnvironment(OUString{ "DBACCESS_HSQL_MIGRATION" }.pData, OUString{ "1" }.pData);
}

struct expect_t
{
    sal_Int16 id;
    OUString number;
};

static const expect_t expect[] = {
    { 1, "0.00" },   { 2, "25.00" },  { 3, "26.00" }, { 4, "30.4" },  { 5, "45.8" },
    { 6, "-25.00" }, { 7, "-26.00" }, { 8, "-30.4" }, { 9, "-45.8" },
};

void Tdf126268Test::testNumbers()
{
    SvtMiscOptions aMiscOptions;
    bool oldValue = aMiscOptions.IsExperimentalMode();

    aMiscOptions.SetExperimentalMode(true);

    // the migration requires the file to be writable
    utl::TempFile const temp(createTempCopy("tdf126268.odb"));
    uno::Reference<XOfficeDatabaseDocument> const xDocument = getDocumentForUrl(temp.GetURL());

    uno::Reference<XConnection> xConnection = getConnectionForDocument(xDocument);

    // select basically everything from the .odb
    uno::Reference<XStatement> statement = xConnection->createStatement();
    const OUString sql{ "SELECT ID, Column1, Column2 FROM tableTest ORDER BY ID" };

    uno::Reference<XResultSet> xRes = statement->executeQuery(sql);
    uno::Reference<XRow> xRow(xRes, UNO_QUERY_THROW);

    // check result
    for (auto& e : expect)
    {
        CPPUNIT_ASSERT(xRes->next());
        CPPUNIT_ASSERT_EQUAL(e.id, xRow->getShort(1));
        CPPUNIT_ASSERT_EQUAL(e.number, xRow->getString(2)); //decimal
        CPPUNIT_ASSERT_EQUAL(e.number, xRow->getString(3)); //numeric
    }
    CPPUNIT_ASSERT(!xRes->next());

    closeDocument(uno::Reference<lang::XComponent>(xDocument, uno::UNO_QUERY));
    if (!oldValue)
        aMiscOptions.SetExperimentalMode(false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Tdf126268Test);

CPPUNIT_PLUGIN_IMPLEMENT();