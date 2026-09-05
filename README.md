# KVD

This is a simple Key/Value store with a simple HTTP-REST-API.

## Compile

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .
make -C build
```

After this you find the binary in the `build/bin/` folder.

## Run

```sh
Usage: kvd [option...]

KVD 0.0.1
(c) 2026 Juergen Mang <mail@jcgames.de>
https://github.com/jcorporation/kvd

Options:
  -h, --help              Displays this help
  -l, --listen <uri>      REST listen URI (default: http://0.0.0.0:8091)
  -o, --loglevel <level>  Syslog loglevel (default: 5 - NOTICE)
  -v, --version           Displays this help
  -w, --workdir <folder>  Working directory (default: /var/lib/kvd)
```

## REST API

| METHOD | PATH | DESCRIPTION |
| ------ | ---- | ----------- |
| DELETE | `/kv1/<key>` | Delete a key. |
| GET | `/kv1/<key>` | Get a value. |
| OPTIONS | `/kv1/<key>` | Get key metadata. |
| PUT | `/kv1/<key>` | Write a key / value pair. |
| POST | `/kv1/<key>` | Same as PUT. |
