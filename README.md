# Sailfish Connect

SailfishOS client for KDE-Connect

## Install

* Openrepos: https://openrepos.net/content/r1tschy/sailfish-connect
* Harbour: *in the future* (when app is stable)

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
    * :heavy_check_mark: Mpris
    * :heavy_check_mark: Ping
    * :construction: Touchpad
    * :construction: SendNotifications
    * :o: FindMyPhone
    * :o: RemoteKeyboard
    * :o: ReceiseNotifications
    * :o: Telephony
    * :o: Share
    * :o: RunCommand
    * :o: Sftp

* UI
    * :heavy_check_mark: Device list
    * :heavy_check_mark: Pairing
    * :heavy_check_mark: Plugin management
    * :heavy_check_mark: Edit device name
    * :construction: SSL key hash view
    * :construction: Device view

* Backend
    * :construction: LAN (some bugs exist)
    * :x: Bluetooth (no QtBluetooth in Harhour allowed)


## Screenshots

![Screenshot 1](/doc/Screenshot_1.png)
![Screenshot 2](/doc/Screenshot_2.png)
![Screenshot 3](/doc/Screenshot_3.png)

## Contribute

* Translations: https://www.transifex.com/r1tschy/sailfish-connect
* Code: You are welcome to make pull request in Github :)

## Known Issues

* Clipboard: Changed clipboard content is only recognized when window is in
   foreground. (#7)
* LAN: Sometimes link gets destroyed right after first connection
   * Symptom: No connection to other device after app start, but connection is possible.
   * Workaround: Do a refresh from Sailfish Connect or KDE Connect.

## Credits

Uses a modified pure-Qt core library from the KDE-Connect desktop client.
