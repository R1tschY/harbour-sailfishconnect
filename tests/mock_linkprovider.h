/*
 * Copyright 2019 Richard Liebscher <richard.liebscher@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOCK_LINKPROVIDER_H
#define MOCK_LINKPROVIDER_H

#include <gmock/gmock.h>

#include <sailfishconnect/backend/linkprovider.h>

class MockLinkProvider : public LinkProvider
{
public:
    MockLinkProvider();
    MockLinkProvider(const QString& name, int priority = 0);

    QString name() override { return m_name; }
    int priority() override { return PRIORITY_HIGH; }

public Q_SLOTS:
    MOCK_METHOD0(onStart, void());
    MOCK_METHOD0(onStop, void());
    MOCK_METHOD1(onNetworkChange, void(const QString&));

private:
    QString m_name;
    int m_priority;
};

#endif // MOCK_LINKPROVIDER_H
