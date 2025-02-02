# VERITAS Eventdisplay Docker Images

Images prepared for each release are available from the [Eventdisplay package registry](https://github.com/VERITAS-Observatory/EventDisplay_v4/pkgs/container/eventdisplay_v4).

## Using the docker image

```console
docker run --rm -it -v "$(pwd):/data" vts-image bash
```

Or to execute directly an Eventdisplay executable:

```console
docker run --rm -it -v "$(pwd):/data" vts-test /opt/Eventdisplay/bin/printRunParameter
```

Note that Eventdisplay is installed into `/opt/Eventdisplay/`

## Building the docker image

Developers can build the docker image locally:

```console
podman build --build-arg NUM_CORES=8 --platform linux/amd64 -f Dockerfile -t evndisplay_v4 .
```
