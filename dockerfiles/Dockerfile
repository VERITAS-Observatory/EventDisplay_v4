ARG ROOT_VERSION=latest
FROM rootproject/root:${ROOT_VERSION} AS vbf_build

LABEL maintainer.name="Eventdisplay Team"
LABEL maintainer.email="gernot.maier@desy.de"

COPY VBF.tar.gz VBF.tar.gz
RUN mkdir VBF &&tar -xzf VBF.tar.gz -C VBF --strip-components=1 \
    && cd VBF \
    && ./configure --prefix=/opt/VBF \
    && make && make install \
    && make clean

FROM rootproject/root:${ROOT_VERSION} as eventdisplay_build
WORKDIR /opt/
COPY --from=vbf_build /opt/VBF /opt/VBF
ENV VBFSYS /opt/VBF
ENV LD_LIBRARY_PATH "${VBFSYS}/lib/:${LD_LIBRARY_PATH}"
ENV PATH ${VBFSYS}/bin:${PATH}

COPY EventDisplay_v4 /opt/Eventdisplay
COPY sofa.tar.gz sofa.tar.gz
ENV EVNDISPSYS /opt/Eventdisplay
ENV LD_LIBRARY_PATH "${EVNDISPSYS}/obj:${EVNDISPSYS}/lib:${ROOTSYS}/lib:${VBFSYS}/lib:${LD_LIBRARY_PATH}"
ENV SOFASYS "${EVNDISPSYS}/sofa"

RUN cd ${EVNDISPSYS} \
    && mv -f /opt/sofa.tar.gz ./sofa/ \
    && ./install_sofa.sh \
    && make VTS

FROM rootproject/root:${ROOT_VERSION}
WORKDIR /opt/
COPY --from=vbf_build /opt/VBF /opt/VBF
COPY --from=eventdisplay_build /opt/Eventdisplay /opt/Eventdisplay

ENV EVNDISPSYS /opt/Eventdisplay
ENV VBFSYS /opt/VBF
ENV LD_LIBRARY_PATH "${VBFSYS}/lib/:${LD_LIBRARY_PATH}"
ENV PATH ${VBFSYS}/bin:${PATH}
ENV LD_LIBRARY_PATH "${EVNDISPSYS}/obj:${EVNDISPSYS}/lib:${ROOTSYS}/lib:${VBFSYS}/lib:${LD_LIBRARY_PATH}"
ENV SOFASYS "${EVNDISPSYS}/sofa"
