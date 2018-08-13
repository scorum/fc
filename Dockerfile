FROM phusion/baseimage:0.9.19

ENV LANG=en_US.UTF-8

RUN \
        apt-get update && \
        apt-get install -y \
            autoconf \
            automake \
            autotools-dev \
            bsdmainutils \
            build-essential \
            cmake \
            doxygen \
            git \
            libboost-all-dev \
            libicu-dev \
            libreadline-dev \
            libssl-dev \
            libtool \
            ncurses-dev \
            pbzip2 \
            pkg-config \
            python3 \
            python3-dev \
            python3-jinja2 \
            python3-pip \
            python-pip \
            nginx \
            fcgiwrap \
            s3cmd \
            awscli \
            jq \
            wget \
            gdb \
        && \
        apt-get clean && \
        rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
        pip3 install gcovr && \
        pip install azure-cli

ADD . /usr/local/src/fc

RUN \
    cd /usr/local/src/fc && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j$(nproc) all

RUN \
    cd /usr/local/src/fc/build && #
    ../tests/tests

CMD ["/bin/ls", "/usr/local/src/fc/build"]