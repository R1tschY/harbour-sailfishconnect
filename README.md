<p align="center">
  <img src="/app/icons/108x108/harbour-sailfishconnect.png" />
</p>

<p align="center">
   <a href="https://github.com/R1tschY/harbour-sailfishconnect/actions?query=workflow%3A%22SailfishConnect+build%22">
      <img src="https://img.shields.io/github/workflow/status/R1tschY/harbour-sailfishconnect/SailfishConnect%20build.svg?style=flat&logo=github" />
   </a>
   <a href="https://openrepos.net/content/r1tschy/sailfish-connect">
      <img src="https://img.shields.io/badge/dynamic/json.svg?color=yellow&label=OpenRepos&query=%24.downloads&url=https%3A%2F%2Fopenrepos.net%2Fapi%2Fv1%2Fapps%2F10437&suffix=+downloads&style=flat&cacheSeconds=3600" />
   </a>
</p>

# Sailfish Connect

Sailfish OS client for [KDE Connect](https://community.kde.org/KDEConnect)

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
Mpris | :heavy_check_mark: | :play_or_pause_button: Control multimedia player from your computer.
Ping | :heavy_check_mark: | :wave: Check connection with a ping.
Touchpad | :heavy_check_mark: | :computer_mouse: Create mouse input from your phone.
Share | :heavy_check_mark: | Send and receive files, text and URLs.
Telephony | :heavy_check_mark: | :telephone_receiver: Share call status (incoming or active) with your computer.
RemoteKeyboard | :heavy_check_mark: | :keyboard: Create keyboard input from your phone.
Contacts | :heavy_check_mark: | :busts_in_silhouette: Share contacts with your computer.
SendNotifications | :construction: | Share notifications of your phone.
RunCommand | :heavy_check_mark: | Run user-defined commands on your computer.
RemoteSystemVolume | :heavy_check_mark: | :sound: Control system volume of your computer.
Telepathy | :construction: | :speech_balloon: Send SMS messages and read SMS history from your computer.
Sftp | :o: | :open_file_folder: Provide access to local files from your computer.
FindMyPhone | :o: | :vibration_mode: Make finding your phone easier.
SystemVolume | :o: | :sound: Share control of system volume.
ReceiseNotifications | :o: | Receive notifications of your computer.
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
* Run in background as service (by default off; activate in settings page)

### Backend
* :heavy_check_mark: LAN
* :x: Bluetooth

## Contribute

* Translations: https://www.transifex.com/r1tschy/sailfish-connect
* Code: You are welcome to make pull request in Github :wink:

## Known Issues

* RemoteKeyboard: Special keys (except modifiers) are interpreted as text by GSConnect
* LAN: A connection through USB-Ethernet and develeper mode is not possible.


## Credits

Sailfish Connect uses libraries of the [KDE Connect desktop client](https://invent.kde.org/network/kdeconnect-kde).
