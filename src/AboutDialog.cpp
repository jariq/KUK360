#include "AboutDialog.h"

#include "Theme.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QStyleHints>
#include <QLabel>
#include <QTextBrowser>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About KUK360"));
    setModal(true);
    resize(740, 460);

    auto *title = new QLabel(QStringLiteral("KUK360 v%1").arg(QCoreApplication::applicationVersion()));
    title->setObjectName(QStringLiteral("aboutTitle"));
    title->setAlignment(Qt::AlignCenter);

    auto *subtitle = new QLabel(tr("Simple 360 photo viewer"));
    subtitle->setObjectName(QStringLiteral("aboutSubtitle"));
    subtitle->setAlignment(Qt::AlignCenter);

    applyTheme();
    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &AboutDialog::applyTheme);

    // The license texts are quoted in a monospace block. %1 is the Qt version.
    const QString html = QStringLiteral(R"html(
<p><b><a href="https://www.kuk360.com/">KUK360</a></b></p>
<p>KUK360 is available under the GNU General Public License version 3:</p>
<blockquote><code>
KUK360 - Simple 360 photo viewer<br/>
Copyright (c) 2019-2026 Jaroslav Imrich &lt;jimrich@jimrich.sk&gt;<br/>
<br/>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 3
as published by the Free Software Foundation.<br/>
<br/>
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.<br/>
<br/>
You should have received a copy of the GNU General Public License
along with this program. If not, see &lt;https://www.gnu.org/licenses/&gt;.
</code></blockquote>

<p>KUK360 uses the third-party components listed below.</p>

<p><b><a href="https://www.qt.io/">Qt Framework</a></b></p>
<p>KUK360 uses the Qt framework %1 with the open-source licensing model:</p>
<blockquote><code>
The Qt Toolkit is Copyright (C) The Qt Company Ltd. and other contributors.
</code></blockquote>
<p>Qt is available under the GNU Lesser General Public License version 3.<br/>
The license texts of Qt and of the third-party code it contains are provided with each distribution.</p>
<p>Qt is linked dynamically and unmodified.<br/>
The complete source code of Qt is available at https://download.qt.io/.</p>

<p><b><a href="https://pannellum.org/">Pannellum</a></b></p>
<p>The panorama renderer and the view interaction are ported from Pannellum:</p>
<blockquote><code>
Copyright (c) 2011-2026 Matthew Petroff<br/>
<br/>
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:<br/>
<br/>
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.<br/>
<br/>
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
</code></blockquote>

<p><b><a href="https://fontawesome.com/v4/">Font Awesome 4.7.0</a></b></p>
<p>Button and menu icons use Font Awesome 4.7.0 by Dave Gandy, licensed under the SIL Open Font License 1.1.</p>
)html").arg(QString::fromLatin1(qVersion()));

    auto *license = new QTextBrowser;
    license->setOpenExternalLinks(true);
    license->setStyleSheet(QStringLiteral("QTextBrowser { font-size: 11pt; }"));
    license->setHtml(html);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 24, 12, 12);
    layout->setSpacing(4);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(20);
    layout->addWidget(license, 1);
}

// Background and heading colors of the design, light or dark after the system scheme.
void AboutDialog::applyTheme()
{
    const Theme::Colors &colors = Theme::colors();
    setStyleSheet(QStringLiteral("AboutDialog { background-color: %1; }"
                                 "#aboutTitle { color: %2; font-size: 16pt; font-weight: bold; }"
                                 "#aboutSubtitle { color: %3; font-size: 11pt; }")
                      .arg(Theme::css(colors.panelBackground), Theme::css(colors.text),
                           Theme::css(colors.secondaryText)));
}
