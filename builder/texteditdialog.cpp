/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "texteditdialog.h"

TextEditDialog::TextEditDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, fl)
{
  setupUi(this);
  if (name)
    setObjectName(name);
  setModal(modal);
}

TextEditDialog::~TextEditDialog()
{
  // no need to delete child widgets, Qt does it all for us
}

void TextEditDialog::languageChange()
{
  retranslateUi(this);
}

