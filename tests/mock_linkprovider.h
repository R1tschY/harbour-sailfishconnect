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
