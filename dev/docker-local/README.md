# Scripts and helpers for building packages and images using dockerized Platform SDK

1. Run `build.sh`
2. To start container build engine run `run.sh`

This image should be used to work with Sailfish OS SDK instead off the virtual machine.

Run `run.sh` to start build engine. It will start container with ssh server running on port `2222` and web server on `8080`.

## Credits

- [CODeRUS](https://github.com/CODeRUS/docker-sailfishos-sdk-local)
- [EvilJazz](https://github.com/evilJazz/sailfishos-buildengine) for the inspiration
- [SfietKonstantin](https://github.com/SfietKonstantin/docker-sailfishos-sdk) for the initial version of scripts
