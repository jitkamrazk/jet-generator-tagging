# Use latest CentOs image as base image
FROM centos:latest
# Prepare dependencies for build
RUN yum groupinstall -y 'Development Tools'
# Install CERN ROOT package and its dependencies
RUN yum install -y epel-release && yum install -y root rsync wget
WORKDIR /opt
# Install Pythia8 model
RUN wget http://home.thep.lu.se/~torbjorn/pythia8/pythia8244.tgz && \
    tar -xvzf pythia8244.tgz
WORKDIR /opt/pythia8244/
RUN ./configure && \
    make && \
    make install
# Install FastJet
WORKDIR /opt
RUN wget http://fastjet.fr/repo/fastjet-3.3.4.tar.gz  && \
    tar -xvzf fastjet-3.3.4.tar.gz && \
    mkdir fastjet
WORKDIR /opt/fastjet-3.3.4/
RUN ./configure --prefix=/opt/fastjet && \
    make && \
    make install
# Set PATH variables
ENV PATH=/opt/pythia8244/bin/:/opt/fastjet/bin/:$PATH
# Compile generator
RUN mkdir /opt/jet-generator-tagging
COPY . /opt/jet-generator-tagging
WORKDIR /opt/jet-generator-tagging
RUN make
ENV PATH=/opt/jet-generator-tagging/bin/:$PATH
RUN mkdir /sim_output && cp -r /opt/jet-generator-tagging/configs /sim_output
WORKDIR /sim_output
ENTRYPOINT ["jet-generator"]