#include "mock_linkprovider.h"

MockLinkProvider::MockLinkProvider()
    : MockLinkProvider(QStringLiteral("MockLinkProvider"))
{ }

MockLinkProvider::MockLinkProvider(const QString& name, int priority)
    : m_name(name)
    , m_priority(priority)
{ }
