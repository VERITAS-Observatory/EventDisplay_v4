ARG ROOT_VERSION=6.30.02-alma9
FROM rootproject/root:${ROOT_VERSION} AS build
ARG VBF_VERSION=0.3.4
ARG BUILD_BRANCH=main
ARG NUM_CORES=4

RUN apt-get update && \
    apt-get install -y --no-install-recommends diffutils && \
    rm -rf /var/lib/apt/lists/*

# VBF
ADD https://syncandshare.desy.de/index.php/s/7i29qcgzgS3zpEk/download /workdir/VBF-$VBF_VERSION.tar.gz
WORKDIR /workdir/
RUN mkdir VBF && tar -xzf VBF-$VBF_VERSION.tar.gz -C VBF --strip-components=1 && \
    cd VBF && \
    ./configure --prefix=/opt/VBF && \
    make && make install && make clean

# Environment (build stage)
ENV VBFSYS=/opt/VBF \
    EVNDISPSYS=/opt/EventDisplay_v4 \
    SOFASYS=/opt/EventDisplay_v4/sofa \
    PATH=/opt/VBF/bin:$PATH \
    LD_LIBRARY_PATH=/opt/EventDisplay_v4/obj:/opt/EventDisplay_v4/lib:$ROOTSYS/lib:/opt/VBF/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

WORKDIR /opt/
RUN BUILD_BRANCH="$(echo "${BUILD_BRANCH}" | sed 's#refs/tags/##')" && \
    git clone -b "${BUILD_BRANCH}" https://github.com/VERITAS-Observatory/EventDisplay_v4.git --depth 1
ADD https://syncandshare.desy.de/index.php/s/RamRFYJtZjDGsfL/download ${SOFASYS}/sofa.tar.gz

WORKDIR /opt/EventDisplay_v4
RUN ./install_sofa.sh && make -j"${NUM_CORES}" VTS

FROM rootproject/root:${ROOT_VERSION}
WORKDIR /opt/
COPY --from=build /opt/VBF /opt/VBF
COPY --from=build /opt/EventDisplay_v4 /opt/EventDisplay_v4

LABEL maintainer.name="Eventdisplay Team" \
    maintainer.email="gernot.maier@desy.de"

# Environment (runtime stage)
ENV EVNDISPSYS=/opt/EventDisplay_v4 \
    VBFSYS=/opt/VBF \
    SOFASYS=/opt/EventDisplay_v4/sofa \
    PATH=/opt/VBF/bin:$PATH \
    LD_LIBRARY_PATH=/opt/EventDisplay_v4/obj:/opt/EventDisplay_v4/lib:$ROOTSYS/lib:/opt/VBF/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} \
    VERITAS_DATA_DIR=/opt/data \
    VERITAS_LOG_DIR=/opt/data \
    VERITAS_USER_DATA_DIR=/opt/user_data \
    VERITAS_USER_LOG_DIR=/opt/user_data
