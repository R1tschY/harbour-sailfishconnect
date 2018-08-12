# harbour-sailfishconnect

SailfishOS client for KDE-Connect

## Features

* Core
    * :heavy_check_mark: Pairing
    * :heavy_check_mark: Plugin management
    * :heavy_check_mark: Use background daemon
    * :construction: Downloads/Uploads (Payload)

* Plugins
    * :heavy_check_mark: Battery
    * :heavy_check_mark: Clipboard
    * :heavy_check_mark: Telepathy
    * :construction: Ping
    * :construction: Mpris
    * :o: FindMyPhone
    * :o: MousePad
    * :o: Notifications
    * :o: ReceiseNotifications
    * :o: RemoteKeyboard
    * :o: RunCommand
    * :o: Sftp
    * :o: Share
    * :o: Telephony

* UI
    * :heavy_check_mark: Device list
    * :heavy_check_mark: Pairing
    * :heavy_check_mark: Plugin management
    * :heavy_check_mark: Edit device name
    * :heavy_check_mark: SSL key hash view
    * :construction: Device view

* Backend
    * :construction: LAN (some bugs exist)
    * :x: Bluetooth (no QtBluetooth in Harhour allowed)


## Screenshots

![Screenshot 1](/doc/Screenshot_1.png)
![Screenshot 2](/doc/Screenshot_2.png)
![Screenshot 3](/doc/Screenshot_3.png)

## Known Issues

    * Clipboard: Changed clipboard content is only recognized when window is in
      foreground. (#7)
    * LAN: Link sometimes gets right destroyed after first connection
        * Symptom: no connection after app start
        * Workaround: Do a refresh from Sailfish Connect or KDE Connect.

## Credits

Uses code from the KDE-Connect desktop client.
