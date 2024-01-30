ARG ROOT_VERSION=6.30.02-alma9
FROM rootproject/root:${ROOT_VERSION} AS build
ARG VBF_VERSION=0.3.4
ARG BUILD_BRANCH=main

LABEL maintainer.name="Eventdisplay Team"
LABEL maintainer.email="gernot.maier@desy.de"

# VBF
ADD https://syncandshare.desy.de/index.php/s/NrnZNKfT8WxFpcn/download /workdir/VBF-$VBF_VERSION.tar.gz
WORKDIR /workdir/
RUN mkdir VBF && tar -xzf VBF-$VBF_VERSION.tar.gz -C VBF --strip-components=1 && \
    cd VBF && \
    ./configure --prefix=/opt/VBF && \
    make && make install && make clean
ENV VBFSYS /opt/VBF
ENV EVNDISPSYS /opt/EventDisplay_v4
ENV SOFASYS "${EVNDISPSYS}/sofa"
ENV PATH ${VBFSYS}/bin:${PATH}
ENV LD_LIBRARY_PATH "${EVNDISPSYS}/obj:${EVNDISPSYS}/lib:${ROOTSYS}/lib:${VBFSYS}/lib:${LD_LIBRARY_PATH}"

WORKDIR /opt/
RUN BUILD_BRANCH=$(echo ${BUILD_BRANCH} | sed 's#refs/tags/##') && \
    git clone -b ${BUILD_BRANCH} https://github.com/VERITAS-Observatory/EventDisplay_v4.git --depth 1
ADD https://syncandshare.desy.de/index.php/s/jr9NrbWFR5MLaDf/download  ${SOFASYS}/sofa.tar.gz

RUN cd ${EVNDISPSYS} \
    && ./install_sofa.sh && make VTS
 
FROM rootproject/root:${ROOT_VERSION}
WORKDIR /opt/
COPY --from=build /opt/VBF /opt/VBF
COPY --from=build /opt/EventDisplay_v4 /opt/EventDisplay_v4
 
ENV EVNDISPSYS /opt/EventDisplay_v4
ENV VBFSYS /opt/VBF
ENV LD_LIBRARY_PATH "${VBFSYS}/lib/:${LD_LIBRARY_PATH}"
ENV PATH ${VBFSYS}/bin:${PATH}
ENV LD_LIBRARY_PATH "${EVNDISPSYS}/obj:${EVNDISPSYS}/lib:${ROOTSYS}/lib:${VBFSYS}/lib:${LD_LIBRARY_PATH}"
ENV SOFASYS "${EVNDISPSYS}/sofa"
 
