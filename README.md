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

KVD 0.0.2
(c) 2026 Juergen Mang <mail@jcgames.de>
https://github.com/jcorporation/kvd

Options:
  -d, --rrdcached <uri>   rrdcached uri (default: empty)
  -h, --help              Displays this help
  -l, --listen <uri>      REST listen URI (default: http://0.0.0.0:8091)
  -o, --loglevel <level>  Syslog loglevel (default: 5 - NOTICE)
  -v, --version           Displays this help
  -w, --workdir <folder>  Working directory (default: /var/lib/kvd)
```

## REST API

| PATH          | METHOD  | DESCRIPTION |
| ------------- | ------- | ----------- |
| `/kv1/`       | GET     | List all keys with metadata |
| `/kv1/<key>`  | DELETE  | Delete a key. |
| `/kv1/<key>`  | GET     | Get a value. |
| `/kv1/<key>`  | OPTIONS | Get key metadata. |
| `/kv1/<key>`  | PUT     | Write a key / value pair. |
| `/kv1/<key>`  | POST    | Same as PUT. |
| `/rrdcached/` | GET     | Lists RRD Cached databases recursively. |
| `/rrdcached/` | POST    | Generic RRD Cached proxy. |
| `/rrdcached/<filename>` | OPTIONS | Sends the INFO command to RRD Cached. |
| `/rrdcached/<filename>` | POST | Sends the FETCH command to RRD Cached. |
| `/rrdcached/<filename>` | PUT | Sends the UPDATE command to RRD Cached. |

## Examples

### RRD Cached Generic

```sh
curl -v http://localhost:8091/rrdcached/ -d@- <<< 'FETCH test.rrd AVERAGE'
```

### RRD Cached Info

```sh
curl -v -X OPTIONS http://localhost:8091/rrdcached/test.rrd
```

### RRD Cached Fetch

```sh
curl -v -X POST http://localhost:8091/rrdcached/test.rrd -d@- <<< 'AVERAGE'
```

### RRD Cached Update

```sh
curl -v -X PUT http://localhost:8091/rrdcached/test.rrd -d@- <<< 'N:12:12'
```
