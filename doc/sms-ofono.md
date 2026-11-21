# Sending SMS through oFono

On Sailfish OS the Telepathy connection for the telephony account is not
usable by unprivileged applications (`CreateChannel` returns "Caller must be
privileged"), so Sailfish Connect sends SMS through oFono directly:

```
org.ofono /ril_<n> org.ofono.MessageManager.SendMessage(number, text) -> o
```

The modem that holds the SIM exposes the `org.ofono.MessageManager`
interface. Both `/ril_0` and `/ril_1` are probed.

## Access control

oFono restricts sensitive methods (`SendMessage`, `Dial`, ...) with
libdbusaccess. Without a configuration file the rule is:

```
* = deny; group(sailfish-radio) | group(privileged) = allow
```

The application runs as `defaultuser`, so sending is denied unless the user is
granted the right to call `SendMessage`. This is the minimal rule
(`/etc/ofono/dbusaccess.conf`):

```
[Common]
DefaultAccess = * = deny; group(sailfish-radio) | group(privileged) = allow

[org.ofono.MessageManager]
SendMessage = * = deny; user(defaultuser) = allow
```

Restart oFono afterwards (`systemctl restart ofono`). Calls stay restricted to
the system groups; only SMS sending is opened to the device user.

## Limitations

Messages are sent but not stored in the commhistory database, so they do not
appear in the phone's Messages application history.
