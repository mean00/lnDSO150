FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_UID=1000
ARG USER_GID=1000
ARG USERNAME=ubuntu

#RUN echo 'Acquire::http::Proxy "http://192.168.0.9:3142";' > /etc/apt/apt.conf.d/01proxy

RUN apt-get update 
RUN apt-get install -y \
    apt-utils \
    bison \
    build-essential \
    bzip2 \
    ca-certificates \
    ccache \
    check \
    cmake \
    curl \
    flex \
    git \
    git-lfs \
    gperf \
    lcov \
    libbsd-dev \
    libffi-dev \
    libglib2.0-0 \
    libncurses-dev \
    libpixman-1-0 \
    libsdl2-2.0-0 \
    libslirp0 \
    libusb-1.0-0-dev \
    make \
    ninja-build \
    python3 \
    python3-venv \
    python3-crcmod \
    ruby \
    sudo \
    unzip \
    usbutils \
    wget \
    xz-utils \
    zip \
  && apt-get autoremove -y \
  && rm -rf /var/lib/apt/lists/* \
  && update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
  && :

RUN rm -f  /etc/apt/apt.conf.d/01proxy
#
#  Reuse Arm & Riscv toolchains
#
RUN mkdir /arm && mkdir /riscv && mkdir /opt/pico && chown ubuntu /opt/pico && chown $USERNAME /riscv /arm
RUN chmod a+rwx /root

CMD [ "/bin/bash" ]

###################################
# Setup user env
###################################
USER ${USERNAME}
RUN mkdir ~/.local &&  mkdir ~/.local/bin &&  mkdir ~/.cargo && mkdir ~/.cargo/bin
RUN echo "export http_proxy=\"\"">> ~/.bashrc
RUN echo "export https_proxy=\"\"">> ~/.bashrc
RUN echo "export HTTP_PROXY=\"\"">> ~/.bashrc
RUN echo "export HTTPS_PROXY=\"\"">> ~/.bashrc
RUN echo "export PATH=\$PATH:\$HOME/.cargo/bin:\$HOME/.local/bin"  >> ~/.bashrc
RUN echo ".  \$HOME/.cargo/env" >> /home/$USERNAME/.bashrc
ENV http_proxy=""
ENV https_proxy=""
ENV HTTP_PROXY=""
ENV HTTPS_PROXY=""


# Arm
RUN  wget -nv --no-proxy -O - https://github.com/mean00/clang_arm_devcontainer/releases/download/v0.1/clang_toolchain_arm_21.1.1_x86_64_ubuntu.24_04.tar.bz2  \
  | bzip2 -dc   | tar -x -C /arm && mv /arm/ATfE-21.1.1-Linux-x86_64   /arm/llvm_21.1 \
            && cd /arm/llvm_21.1/bin && ln -s clang++ clang++-21
#


