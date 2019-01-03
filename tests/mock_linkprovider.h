#ifndef MOCK_LINKPROVIDER_H
#define MOCK_LINKPROVIDER_H

#include <gmock/gmock.h>

#include <sailfishconnect/backend/linkprovider.h>

class MockLinkProvider : public LinkProvider
{
public:
    MockLinkProvider();

    MOCK_METHOD0(name, QString());
    MOCK_METHOD0(priority, int());

public Q_SLOTS:
    MOCK_METHOD0(onStart, void());
    MOCK_METHOD0(onStop, void());
    MOCK_METHOD1(onNetworkChange, void(const QString&));
};

#endif // MOCK_LINKPROVIDER_H
