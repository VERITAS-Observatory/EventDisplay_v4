ARG ROOT_VERSION=6.38.00-ubuntu25.10
FROM rootproject/root:${ROOT_VERSION} AS build
ARG VBF_VERSION=0.3.4
ARG BUILD_BRANCH=main
ARG NUM_CORES=4
# SOFA version must match SOFAD in install_sofa.sh (currently 20210512)

RUN apt-get update && \
    apt-get install -y --no-install-recommends diffutils && \
    rm -rf /var/lib/apt/lists/*

# VBF
ADD https://github.com/VERITAS-Observatory/VBF.git#$VBF_VERSION-c++17 /workdir/VBF
WORKDIR /workdir/
RUN cd VBF && \
    ./configure --prefix=/opt/VBF && \
    make && make install && make clean

# Environment (build stage)
ENV LD_LIBRARY_PATH=""
ENV VBFSYS=/opt/VBF \
    EVNDISPSYS=/opt/EventDisplay_v4 \
    SOFASYS=/opt/EventDisplay_v4/sofa \
    PATH=/opt/VBF/bin:$PATH \
    LD_LIBRARY_PATH=/opt/EventDisplay_v4/obj:/opt/EventDisplay_v4/lib:$ROOTSYS/lib:/opt/VBF/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}

WORKDIR /opt/
RUN BUILD_BRANCH="$(echo "${BUILD_BRANCH}" | sed 's#refs/tags/##')" && \
    git clone -b "${BUILD_BRANCH}" https://github.com/VERITAS-Observatory/EventDisplay_v4.git --depth 1
ADD https://www.iausofa.org/2021_0512_C/sofa_c-20210512.tar.gz ${SOFASYS}/sofa.tar.gz

WORKDIR /opt/EventDisplay_v4
RUN ./install_sofa.sh && make -j"${NUM_CORES}" VTS

FROM rootproject/root:${ROOT_VERSION}
WORKDIR /opt/
COPY --from=build /opt/VBF /opt/VBF
COPY --from=build /opt/EventDisplay_v4 /opt/EventDisplay_v4

LABEL maintainer.name="Eventdisplay Team" \
    maintainer.email="gernot.maier@desy.de"

# Environment (runtime stage)
ENV LD_LIBRARY_PATH=""
ENV EVNDISPSYS=/opt/EventDisplay_v4 \
    VBFSYS=/opt/VBF \
    SOFASYS=/opt/EventDisplay_v4/sofa \
    PATH=/usr/local/bin:/opt/VBF/bin:$PATH \
    LD_LIBRARY_PATH=/opt/EventDisplay_v4/obj:/opt/EventDisplay_v4/lib:$ROOTSYS/lib:/opt/VBF/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH} \
    VERITAS_DATA_DIR=/opt/data \
    VERITAS_LOG_DIR=/opt/data \
    VERITAS_USER_DATA_DIR=/opt/user_data \
    VERITAS_USER_LOG_DIR=/opt/user_data
