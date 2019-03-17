#!/bin/sh

cleanup()
{
    pkill -KILL -f "/usr/bin/ruby /usr/bin/puma -p 8080 -t 1:1 -e production"
}
trap "cleanup; exit" INT QUIT TERM EXIT

cd /usr/lib/sdk-webapp-bundle
/usr/bin/ruby /usr/bin/puma -p 8080 -t 1:1 -e production &

sudo /usr/sbin/sshd -p 2222 -D -e -f /etc/ssh/sshd_config_engine
