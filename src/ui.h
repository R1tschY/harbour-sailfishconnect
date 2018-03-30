#ifndef UI_H
#define UI_H

#include <QObject>
#include <QString>

class QQuickView;

namespace SailfishConnect {

class AppDaemon;

/**
 * @brief manages ui
 *
 * - dbus interface to ui
 * - creates the QQuickView with the main window
 */
class UI : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.harbour.SailfishConnect.UI")
public:
    static const QString DBUS_INTERFACE_NAME;
    static const QString DBUS_PATH;

    explicit UI(AppDaemon* daemon, QObject *parent = nullptr);

    /**
     * @brief notify other main daemon to show app window
     */
    static void raise();

public slots:
    /**
     * @brief show the main window
     */
    Q_SCRIPTABLE void showMainWindow();

private:
    QQuickView* m_view = nullptr;
    AppDaemon *m_daemon;

    void onMainWindowDestroyed();

    bool eventFilter(QObject *obj, QEvent *event) override;
};

} // namespace SailfishConnect

#endif // UI_H
