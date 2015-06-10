/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "toolchainconfigwidget.h"
#include "gotoolchain.h"

#include <utils/qtcassert.h>
#include <utils/fileutils.h>

#include <QString>

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>

namespace GoLang {

ToolChainConfigWidget::ToolChainConfigWidget(Internal::GoToolChain *tc) :
    m_mainLayout(0),
    m_nameLineEdit(0),
    m_compilerPathLineEdit(0),
    m_goRootEdit(0),
    m_toolChain(tc),
    m_errorLabel(0)
{
    QTC_CHECK(tc);
    m_nameLineEdit = new QLineEdit(this);
    m_compilerPathLineEdit = new QLineEdit(this);
    m_goRootEdit = new QLineEdit(this);
    m_mainLayout = new QFormLayout(this);
    m_mainLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow); // for the Macs...
    m_mainLayout->addRow(tr("Name:"), m_nameLineEdit);
    m_mainLayout->addRow(tr("Compiler:"), m_compilerPathLineEdit);
    m_mainLayout->addRow(tr("Go root directory:"), m_goRootEdit);

    discard();
    connect(m_nameLineEdit, SIGNAL(textChanged(QString)), SIGNAL(dirty()));
    connect(m_compilerPathLineEdit, SIGNAL(textChanged(QString)), SIGNAL(dirty()));
    connect(m_goRootEdit, SIGNAL(textChanged(QString)), SIGNAL(dirty()));
}

void ToolChainConfigWidget::apply()
{
    m_toolChain->setDisplayName(m_nameLineEdit->text());
    m_toolChain->setCompilerCommand(Utils::FileName::fromUserInput(m_compilerPathLineEdit->text()),
                                    Utils::FileName::fromUserInput(m_goRootEdit->text()));
}

void ToolChainConfigWidget::discard()
{
    m_nameLineEdit->setText(m_toolChain->displayName());
    m_compilerPathLineEdit->setText(m_toolChain->compilerCommand().toUserOutput());
    m_goRootEdit->setText(m_toolChain->goRoot().toUserOutput());
}

bool ToolChainConfigWidget::isDirty() const
{
    return m_nameLineEdit->text() != m_toolChain->displayName();
}

Internal::GoToolChain *ToolChainConfigWidget::toolChain() const
{
    return m_toolChain;
}

void ToolChainConfigWidget::makeReadOnly()
{
    m_nameLineEdit->setEnabled(false);
    m_compilerPathLineEdit->setEnabled(false);
    m_goRootEdit->setEnabled(false);
}

void ToolChainConfigWidget::addErrorLabel()
{
    if (!m_errorLabel) {
        m_errorLabel = new QLabel;
        m_errorLabel->setVisible(false);
    }
    m_mainLayout->addRow(m_errorLabel);
}

void ToolChainConfigWidget::setErrorMessage(const QString &m)
{
    QTC_ASSERT(m_errorLabel, return);
    if (m.isEmpty()) {
        clearErrorMessage();
    } else {
        m_errorLabel->setText(m);
        m_errorLabel->setStyleSheet(QLatin1String("background-color: \"red\""));
        m_errorLabel->setVisible(true);
    }
}

void ToolChainConfigWidget::clearErrorMessage()
{
    QTC_ASSERT(m_errorLabel, return);
    m_errorLabel->clear();
    m_errorLabel->setStyleSheet(QString());
    m_errorLabel->setVisible(false);
}

} // namespace GoLang
