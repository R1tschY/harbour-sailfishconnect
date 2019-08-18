<p align="center">
  <img src="/app/icons/108x108/harbour-sailfishconnect.png" />
</p>

<p align="center">
   <a href="https://circleci.com/gh/R1tschY/harbour-sailfishconnect">
      <img src="https://img.shields.io/circleci/project/github/R1tschY/harbour-sailfishconnect.svg?style=flat&logo=circleci" />
   </a>
   <a href="https://openrepos.net/content/r1tschy/sailfish-connect">
      <img src="https://img.shields.io/badge/dynamic/json.svg?color=yellow&label=OpenRepos&query=%24.downloads&url=https%3A%2F%2Fopenrepos.net%2Fapi%2Fv1%2Fapps%2F10437&suffix=+downloads&style=flat&cacheSeconds=3600" />
   </a>
</p>

# Sailfish Connect

An alternative Sailfish OS client for [KDE Connect](https://community.kde.org/KDEConnect)



## Screenshots

![Screenshot 1](/doc/Screenshot_1.png)
![Screenshot 2](/doc/Screenshot_2.png)
![Screenshot 3](/doc/Screenshot_3.png)

## Install

* Openrepos: https://openrepos.net/content/r1tschy/sailfish-connect
* Harbour: *in the future* (when app is stable)

## KDE Connect Clients

* [KDE Connect KDE](https://github.com/KDE/kdeconnect-kde): Desktop client mainly written for KDE
* [KDE Connect Android](https://f-droid.org/packages/org.kde.kdeconnect_tp)
* [GSConnect](https://extensions.gnome.org/extension/1319/gsconnect/): Desktop client written for GNOME Shell

## Features

### Plugins

Name | Status | Description
-----|--------|------------
Battery | :heavy_check_mark: | :battery: Share battery status with your computer.
Clipboard | :heavy_check_mark: | :clipboard: Share clipboard text content.
Telepathy | :heavy_check_mark: | :speech_balloon: Send SMS messages from your computer.
Mpris | :heavy_check_mark: | :play_or_pause_button: Control multimedia player from your computer.
Ping | :heavy_check_mark: | :wave: Check connection with a ping.
Touchpad | :heavy_check_mark: | :computer_mouse: Create mouse input from your phone.
Share | :heavy_check_mark: | Send and receive files, text and URLs.
Telephony | :heavy_check_mark: | :telephone_receiver: Share call status (incoming or active) with your computer.
RemoteKeyboard | :heavy_check_mark: | :keyboard: Create keyboard input from your phone.
SendNotifications | :construction: | Share notifications of your phone.
RunCommand | :o: | Run user-defined commands on your computer.
Sftp | :o: | :open_file_folder: Provide access to local files from your computer.
FindMyPhone | :o: | :vibration_mode: Make finding your phone easier.
SystemVolume | :o: | :sound: Share control of system volume.
ReceiseNotifications | :o: | Receive notifications of your computer.
Contacts | :o: | :busts_in_silhouette: Share contacts with your computer.
MprisReceiver | :o: | :play_or_pause_button: Share multimedia player control with your computer.


### Core
* Pairing
* Plugin management
* Use background daemon
* Downloads/Uploads (Payload)

### UI
* Device list
* Pairing
* Plugin management
* SSL key hash view
* Device view
* Show progress of file transfers

### Backend
* :heavy_check_mark: LAN
* :x: Bluetooth (no QtBluetooth in Harhour allowed)

## Contribute

* Translations: https://www.transifex.com/r1tschy/sailfish-connect
* Code: You are welcome to make pull request in Github :wink:

## Known Issues

* SendNotifications: GSConnect does not show some of the notifications
* LAN: A connection through USB-Ethernet and develeper mode is not possible.
* LAN: Sometimes link gets destroyed right after first connection
   * Symptom: No connection to other device after app start, but connection is possible.
   * Workaround: Do a refresh from Sailfish Connect or KDE Connect.

## Credits

Sailfish Connect uses an own fork of the core library of the
[KDE Connect desktop client](https://cgit.kde.org/kdeconnect-kde.git/).
It was ported to use only Qt.
