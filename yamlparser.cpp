/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2013 Razor team
 * Authors:
 *   Christian Surlykke <christian@surlykke.dk>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include <QIODevice>
#include <QRegularExpression>
#include <QDebug>

#include <LXQt/Globals>

#include "yamlparser.h"

YamlParser::YamlParser()
{
    state = start;
}

YamlParser::~YamlParser()
{
}

void YamlParser::consumeLine(const QString &line)
{
    static QRegularExpression documentStart(QSL("---\\s*(\\[\\]\\s*)?"));
    static QRegularExpression mapStart(QSL("(-\\s*)(\\w*)\\s*:(.*)$"));
    static QRegularExpression mapEntry(QSL("(\\s*)(\\w*)\\s*:(.*)"));
    static QRegularExpression continuation(QSL("(\\s*)(.*)"));
    static QRegularExpression documentEnd(QSL("...\\s*"));
    static QRegularExpression emptyLine(QSL("\\s*(#.*)?"));

    QString anchoredLine = QRegularExpression::anchoredPattern(line);
    QRegularExpressionMatch regexMatch;

    //qDebug() << line;

    if (documentStart.match(anchoredLine).hasMatch())
    {
        m_ListOfMaps.clear();
        state = atdocumentstart;
        m_CurrentIndent = -1;
    }
    else if (state == error)
    {
        // Skip
    }
    else if (emptyLine.match(anchoredLine).hasMatch())
    {
        // Skip
    }
    else if ((state == atdocumentstart || state == inlist)
             && (regexMatch = mapStart.match(anchoredLine)).hasMatch())
    {
        m_ListOfMaps << QMap<QString, QString>();
        addEntryToCurrentMap(regexMatch.captured(2), regexMatch.captured(3));
        m_CurrentIndent = regexMatch.captured(1).size();
        state = inlist;
    }
    else if (state == inlist
             && (regexMatch = mapEntry.match(anchoredLine)).hasMatch()
             && regexMatch.captured(1).size() == m_CurrentIndent)
    {
        addEntryToCurrentMap(regexMatch.captured(2), regexMatch.captured(3));
    }
    else if (state == inlist
             && (regexMatch = continuation.match(anchoredLine)).hasMatch()
             && regexMatch.captured(1).size() > m_CurrentIndent)
    {
        m_ListOfMaps.last()[m_LastKey].append(regexMatch.captured(2));
    }
    else if ((state == atdocumentstart || state == inlist) && documentEnd.match(anchoredLine).hasMatch())
    {
        qDebug() << "emitting:" << m_ListOfMaps;
        emit newListOfMaps(m_ListOfMaps);
        state = documentdone;
    }
    else
    {
        qWarning() << "Yaml parser could not read:" << line;
        state = error;
    }
}

void YamlParser::addEntryToCurrentMap(QString key, QString value)
{
    m_ListOfMaps.last()[key.trimmed()] = value.trimmed();
    m_LastKey = key.trimmed();
}
