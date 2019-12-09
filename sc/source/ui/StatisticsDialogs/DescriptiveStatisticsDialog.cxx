/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <reffact.hxx>
#include <TableFillingAndNavigationTools.hxx>
#include <DescriptiveStatisticsDialog.hxx>
#include <scresid.hxx>
#include <strings.hrc>

namespace
{

struct StatisticCalculation {
    const char* aCalculationNameId;
    const char* aFormula;
};

static const StatisticCalculation lclCalcDefinitions[] =
{
    { STRID_CALC_MEAN,           "=AVERAGE(%RANGE%)" },
    { STRID_CALC_STD_ERROR,      "=SQRT(VAR(%RANGE%)/COUNT(%RANGE%))"},
    { STRID_CALC_MODE,           "=MODE(%RANGE%)"},
    { STRID_CALC_MEDIAN,         "=MEDIAN(%RANGE%)"},
    { STRID_CALC_FIRST_QUARTILE, "=QUARTILE(%RANGE%; 1)" },
    { STRID_CALC_THIRD_QUARTILE, "=QUARTILE(%RANGE%; 3)" },
    { STRID_CALC_VARIANCE,       "=VAR(%RANGE%)"},
    { STRID_CALC_STD_DEVIATION,  "=STDEV(%RANGE%)"},
    { STRID_CALC_KURTOSIS,       "=KURT(%RANGE%)"},
    { STRID_CALC_SKEWNESS,       "=SKEW(%RANGE%)"},
    { STRID_CALC_RANGE,          "=MAX(%RANGE%)-MIN(%RANGE%)"},
    { STRID_CALC_MIN,            "=MIN(%RANGE%)"},
    { STRID_CALC_MAX,            "=MAX(%RANGE%)"},
    { STRID_CALC_SUM,            "=SUM(%RANGE%)"},
    { STRID_CALC_COUNT,          "=COUNT(%RANGE%)" },
    { nullptr,                   nullptr }
};

}

ScDescriptiveStatisticsDialog::ScDescriptiveStatisticsDialog(
                    SfxBindings* pSfxBindings, SfxChildWindow* pChildWindow,
                    weld::Window* pParent, ScViewData* pViewData ) :
    ScStatisticsInputOutputDialog(
            pSfxBindings, pChildWindow, pParent, pViewData,
            "modules/scalc/ui/descriptivestatisticsdialog.ui",
            "DescriptiveStatisticsDialog")
{}

ScDescriptiveStatisticsDialog::~ScDescriptiveStatisticsDialog()
{}

void ScDescriptiveStatisticsDialog::Close()
{
    DoClose( ScDescriptiveStatisticsDialogWrapper::GetChildWindowId() );
}

const char* ScDescriptiveStatisticsDialog::GetUndoNameId()
{
    return STR_DESCRIPTIVE_STATISTICS_UNDO_NAME;
}

ScRange ScDescriptiveStatisticsDialog::ApplyOutput(ScDocShell* pDocShell)
{
    AddressWalkerWriter aOutput(mOutputAddress, pDocShell, mDocument,
            formula::FormulaGrammar::mergeToGrammar( formula::FormulaGrammar::GRAM_ENGLISH, mAddressDetails.eConv));
    FormulaTemplate aTemplate(mDocument);

    std::unique_ptr<DataRangeIterator> pIterator;
    if (mGroupedBy == BY_COLUMN)
        pIterator.reset(new DataRangeByColumnIterator(mInputRange));
    else
        pIterator.reset(new DataRangeByRowIterator(mInputRange));

    aOutput.nextColumn();

    // Use explicit sheet name in case the input and output are on different sheets.
    bool b3DAddress = mInputRange.aStart.Tab() != mOutputAddress.Tab();

    // Write column/row labels
    for( ; pIterator->hasNext(); pIterator->next() )
    {
        if (mGroupedBy == BY_COLUMN)
            aTemplate.setTemplate(ScResId(STR_COLUMN_LABEL_TEMPLATE));
        else
            aTemplate.setTemplate(ScResId(STR_ROW_LABEL_TEMPLATE));

        aTemplate.applyNumber("%NUMBER%", pIterator->index() + 1);
        aOutput.writeBoldString(aTemplate.getTemplate());
        aOutput.nextColumn();
    }
    aOutput.nextRow();
    aOutput.resetColumn();
    aOutput.push();

    // Write calculation labels
    for(sal_Int32 i = 0; lclCalcDefinitions[i].aFormula != nullptr; i++)
    {
        OUString aLabel(ScResId(lclCalcDefinitions[i].aCalculationNameId));
        aOutput.writeString(aLabel);
        aOutput.nextRow();
    }
    aOutput.nextColumn();

    pIterator->reset();

    for( ; pIterator->hasNext(); pIterator->next() )
    {
        aOutput.resetRow();

        for(sal_Int32 i = 0; lclCalcDefinitions[i].aFormula != nullptr; i++)
        {
            aTemplate.setTemplate(lclCalcDefinitions[i].aFormula);
            aTemplate.applyRange("%RANGE%", pIterator->get(), b3DAddress);
            aOutput.writeFormula(aTemplate.getTemplate());
            aOutput.nextRow();
        }
        aOutput.nextColumn();
    }

    return ScRange(aOutput.mMinimumAddress, aOutput.mMaximumAddress);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */