# syntax=docker/dockerfile:1
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake ninja-build pkg-config git curl \
    gcc-13 g++-13 \
    libgl1-mesa-dev libglu1-mesa-dev libglew-dev \
    libx11-dev libxi-dev libxrandr-dev libxinerama-dev \
    libxcursor-dev libxxf86vm-dev \
    qtbase5-dev qtbase5-dev-tools qtbase5-private-dev \
    libqt5opengl5-dev \
    liblua5.4-dev \
    libmysqlclient-dev \
    libmysqlcppconn-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# CMakeLists searches for "libmysql" (Windows name) - tell cmake exactly where
# the Linux equivalents are so USE_SQL=ON doesn't FATAL_ERROR
RUN MYSQL_LIB=$(find /usr/lib -name "libmysqlclient.so" | head -1) && \
    CPPCONN_LIB=$(find /usr/lib -name "libmysqlcppconn.so" | head -1) && \
    CPPCONN_INC=$(find /usr/include -name "driver.h" -path "*/cppconn/*" | xargs dirname | xargs dirname | head -1) && \
    echo "MySQL lib:    $MYSQL_LIB" && \
    echo "CppConn lib:  $CPPCONN_LIB" && \
    echo "CppConn inc:  $CPPCONN_INC" && \
    cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=gcc-13 \
        -DCMAKE_CXX_COMPILER=g++-13 \
        -DCMAKE_CXX_FLAGS="-Wno-error -Wno-unused -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated -Wno-deprecated-declarations" \
        -DUSE_SQL=ON \
        -DMYSQL_LIBRARY="$MYSQL_LIB" \
        -DMYSQLCPPCONN_LIBRARY="$CPPCONN_LIB" \
        -DMYSQLCPPCONN_INCLUDE="$CPPCONN_INC" \
        -DNOGGIT_WITH_SCRIPTING=ON \
        -DNOGGIT_OPENGL_ERROR_CHECK=ON \
        -DNOGGIT_BINDLESS_TEXTURES=ON \
        -DVALIDATE_OPENGL_PROGRAMS=ON

RUN bash -c 'cmake --build build -j$(nproc) 2>&1 | tee /tmp/build.log; \
             ret=${PIPESTATUS[0]}; \
             grep -E " error:" /tmp/build.log | grep -v "warning:" | head -40; \
             exit $ret'
