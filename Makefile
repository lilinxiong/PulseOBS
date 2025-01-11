# Variables
CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = PulseOBS

# Default target to build the whole project
all: build

# Build the whole project
build:
	mkdir -p build
	cd build && cmake .. && cmake --build .

# Custom target to build Docker image
docker-build:
	docker build -t pulse-obs .

# Custom target to run Docker container
docker-run:
	docker run -it --rm -v .:/app pulse-obs

# Clean build files
clean:
	rm -rf build
	rm -f $(TARGET)