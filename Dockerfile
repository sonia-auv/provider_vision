ARG BASE_IMAGE="ghcr.io/sonia-auv/sonia_common/sonia_common:x86-perception-latest"

FROM ${BASE_IMAGE}

USER root

ARG BUILD_DATE
ARG VERSION

ARG TARGET_ARCH="x86"

ENV NODE_NAME=provider_vision

LABEL net.etsmtl.sonia-auv.node.build-date=${BUILD_DATE}
LABEL net.etsmtl.sonia-auv.node.version=${VERSION}
LABEL net.etsmtl.sonia-auv.node.name=${NODE_NAME}


ENV SONIA_WS=${SONIA_HOME}/ros_sonia_ws

ENV NODE_NAME=${NODE_NAME}
ENV NODE_PATH=${SONIA_WS}/src/${NODE_NAME}
ENV LAUNCH_FILE=${NODE_NAME}.launch
ENV SCRIPT_DIR=${SONIA_WS}/scripts
ENV ENTRYPOINT_FILE=sonia_entrypoint.sh
ENV LAUNCH_ABSPATH=${NODE_PATH}/launch/${LAUNCH_FILE}
ENV ENTRYPOINT_ABSPATH=${NODE_PATH}/scripts/${ENTRYPOINT_FILE}

ENV SONIA_WS_SETUP=${SONIA_WS}/devel/setup.bash

RUN apt-get update \ 
    && apt-get install -y libunwind-dev ros-noetic-cv-bridge ros-noetic-image-transport qt5-default

WORKDIR ${NODE_PATH}/drivers/${TARGET_ARCH}

#Copy the drivers to install the spinnaker sdk
COPY ./drivers/${TARGET_ARCH} .

RUN chmod +x install_spinnaker.sh \
     && sh install_spinnaker.sh < input

RUN bash -c "source /etc/profile.d/setup_flir_gentl_64.sh 64"

ENV FLIR_GENTL64_CTI=/opt/spinnaker/lib/flir-gentl/FLIR_GenTL.cti

WORKDIR ${SONIA_WS}

#Copy the code of provider_vision and the rest (changes more often then the sdk)
COPY . ${NODE_PATH}

RUN bash -c "source ${ROS_WS_SETUP}; source ${BASE_LIB_WS_SETUP}; catkin_make"

RUN chown -R ${SONIA_USER}: ${SONIA_WS}
RUN usermod -a -G root ${SONIA_USER}
USER ${SONIA_USER}

RUN mkdir ${SCRIPT_DIR}
RUN cat $ENTRYPOINT_ABSPATH > ${SCRIPT_DIR}/entrypoint.sh
RUN echo "roslaunch --wait $LAUNCH_ABSPATH" > ${SCRIPT_DIR}/launch.sh

RUN chmod +x ${SCRIPT_DIR}/entrypoint.sh && chmod +x ${SCRIPT_DIR}/launch.sh

RUN echo "source $SONIA_WS_SETUP" >> ~/.bashrc

ENTRYPOINT ["./scripts/entrypoint.sh"]
CMD ["./scripts/launch.sh"]
