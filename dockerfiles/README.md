# VERITAS Eventdisplay Docker Images

Prepared for each release.

## Using the docker image

```console
docker run --rm -it -v "$(pwd):/data" vts-image bash
```

Or to execute directly an Eventdisplay executable:

```console
docker run --rm -it -v "$(pwd):/data" vts-test /opt/Eventdisplay/bin/printRunParameter
```

Note that Eventdisplay is installed into `/opt/Eventdisplay/`
