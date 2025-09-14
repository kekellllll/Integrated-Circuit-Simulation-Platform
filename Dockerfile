# Use Ubuntu 20.04 as base image
FROM ubuntu:20.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    nodejs \
    npm \
    && rm -rf /var/lib/apt/lists/*

# Install CUDA (optional - comment out if not needed)
# RUN wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-keyring_1.0-1_all.deb \
#     && dpkg -i cuda-keyring_1.0-1_all.deb \
#     && apt-get update \
#     && apt-get install -y cuda-toolkit-11-8 \
#     && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build C++ components
RUN mkdir -p build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release .. \
    && make -j$(nproc)

# Build web interface
RUN cd web && npm install && npm run build

# Create plugins directory
RUN mkdir -p /app/plugins

# Set up runtime environment
ENV PATH="/app/build/bin:${PATH}"
ENV LD_LIBRARY_PATH="/app/build/lib:${LD_LIBRARY_PATH}"

# Expose web server port
EXPOSE 8080

# Create entrypoint script
RUN echo '#!/bin/bash\n\
echo "=== Integrated Circuit Simulation Platform ==="\n\
echo "Starting simulation server..."\n\
\n\
# Start web server in background\n\
cd /app/web && python3 -m http.server 8080 &\n\
WEB_PID=$!\n\
\n\
# Start simulation engine\n\
cd /app/build\n\
./bin/ic_simulator --web-mode\n\
\n\
# Keep container running\n\
wait $WEB_PID\n\
' > /app/entrypoint.sh && chmod +x /app/entrypoint.sh

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/ || exit 1

# Run the application
CMD ["/app/entrypoint.sh"]