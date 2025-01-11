# Use an image with cmake as a base
FROM danger89/cmake:latest

# Set the working directory inside the container
WORKDIR /app

# Copy the project files into the container
COPY . .

# Install build-essential package for C++ development
RUN apt-get update -y
RUN apt-get install -y build-essential
RUN rm -rf /var/lib/apt/lists/*

# Default command to run in the container
CMD ["bash"]
