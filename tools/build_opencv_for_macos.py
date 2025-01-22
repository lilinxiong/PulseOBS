import os
import subprocess
from enum import Enum
import shutil


# get cpu core size
num_cores = os.cpu_count()

print(f"cpu core numbers: {num_cores}")

current_file_path = os.path.abspath(__file__)

print('current_file_path:', current_file_path)

current_directory_path = os.path.dirname(current_file_path)
parent_directory_path = os.path.dirname(current_directory_path)

third_party_directory_path = os.path.join(parent_directory_path, '3rdparty')
binary_path = os.path.join(third_party_directory_path, 'binary')

opencv_src_path = os.path.join(third_party_directory_path, 'opencv-4.11.0')

print('opencv_src_path:', opencv_src_path)

opencv_out_dir_root = os.path.join(binary_path, 'opencv-macos')
print('opencv_out_dir_root:', opencv_out_dir_root)

if not os.path.exists(opencv_out_dir_root):
    os.makedirs(opencv_out_dir_root)
else:
    print(f"Directory {opencv_out_dir_root} already exists. Deleting it.")
    shutil.rmtree(opencv_out_dir_root)
    os.makedirs(opencv_out_dir_root)


opencv_tmp_build_dir_root = os.path.join(opencv_src_path, '.build')
print('opencv_tmp_build_dir_root:', opencv_tmp_build_dir_root)

if not os.path.exists(opencv_tmp_build_dir_root):
    os.makedirs(opencv_tmp_build_dir_root)
else:
    print(f"Directory {opencv_tmp_build_dir_root} already exists. Deleting it.")
    shutil.rmtree(opencv_tmp_build_dir_root)
    os.makedirs(opencv_tmp_build_dir_root)

# cd to OpenCV build tmp dir....
os.chdir(opencv_tmp_build_dir_root)

opencv_cmake_configure_cmd = [
    "cmake",
    f"-DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'",
    f"-DCMAKE_BUILD_TYPE=Release",
    f"-DCMAKE_INSTALL_PREFIX={opencv_out_dir_root}",
    f"-DBUILD_CUDA_STUBS=OFF",
    f"-DBUILD_DOCS=OFF",
    f"-DBUILD_EXAMPLES=OFF",
    f"-DBUILD_FAT_JAVA_LIB=OFF",
    f"-DBUILD_ITT=OFF",
    f"-DBUILD_JASPER=OFF",
    f"-DBUILD_JAVA=OFF",
    f"-DBUILD_JPEG=OFF",
    f"-DBUILD_OPENEXR=OFF",
    f"-DBUILD_OPENJPEG=OFF",
    f"-DBUILD_PERF_TESTS=OFF",
    f"-DBUILD_PNG=OFF",
    f"-DBUILD_PROTOBUF=OFF",
    f"-DBUILD_SHARED_LIBS=OFF",
    f"-DBUILD_TBB=OFF",
    f"-DBUILD_TESTS=OFF",
    f"-DBUILD_TIFF=OFF",
    f"-DBUILD_WEBP=OFF",
    f"-DBUILD_ZLIB=OFF",
    f"-DBUILD_opencv_apps=OFF",
    f"-DBUILD_opencv_dnn=OFF",
    f"-DBUILD_opencv_java_bindings_generator=OFF",
    f"-DBUILD_opencv_js_bindings_generator=OFF",
    f"-DBUILD_opencv_objc_bindings_generator=OFF",
    f"-DBUILD_opencv_python_bindings_generator=OFF",
    f"-DBUILD_opencv_python_tests=OFF",
    f"-DBUILD_opencv_ts=OFF",
    f"-DBUILD_opencv_world=OFF",
    f"-DCPU_BASELINE=NEON_DOTPROD",
    f"-DCPU_DISPATCH=NEON_DOTPROD",
    f"-DENABLE_LIBJPEG_TURBO_SIMD=OFF",
    f"-DOPENCL_FOUND=OFF",
    f"-DOPENCV_DNN_CUDA=OFF",
    f"-DOPENCV_DNN_OPENCL=OFF",
    f"-DOPENCV_DNN_OPENVINO=OFF",
    f"-DOPENCV_DNN_PERF_CAFFE=OFF",
    f"-DOPENCV_DNN_TFLITE=OFF",
    f"-DOPENCV_TEST_DNN_TFLITE=OFF",
    f"-DPNG_ARM_NEON=OFF",
    f"-DWITH_ADE=OFF",
    f"-DWITH_AVFOUNDATION=OFF",
    f"-DWITH_AVIF=OFF",
    f"-DWITH_CANN=OFF",
    f"-DWITH_EIGEN=OFF",
    f"-DWITH_CLP=OFF",
    f"-DWITH_CUDA=OFF",
    f"-DWITH_FFMPEG=OFF",
    f"-DWITH_FLATBUFFERS=OFF",
    f"-DWITH_FREETYPE=OFF",
    f"-DWITH_GDAL=OFF",
    f"-DWITH_GDCM=OFF",
    f"-DWITH_GPHOTO2=OFF",
    f"-DWITH_GSTREAMER=OFF",
    f"-DWITH_HALIDE=OFF",
    f"-DWITH_IMGCODEC_GIF=OFF",
    f"-DWITH_IMGCODEC_HDR=OFF",
    f"-DWITH_ITT=OFF",
    f"-DWITH_JASPER=OFF",
    f"-DWITH_JPEG=OFF",
    f"-DWITH_JPEGXL=OFF",
    f"-DWITH_ONNX=OFF",
    f"-DWITH_OPENCL=OFF",
    f"-DWITH_OPENCLAMDBLAS=OFF",
    f"-DWITH_OPENCLAMDFFT=OFF",
    f"-DWITH_OPENCL_SVM=OFF",
    f"-DWITH_OPENEXR=OFF",
    f"-DWITH_OPENGL=OFF",
    f"-DWITH_OPENJPEG=OFF",
    f"-DWITH_OPENMP=OFF",
    f"-DWITH_OPENNI=OFF",
    f"-DWITH_OPENNI2=OFF",
    f"-DWITH_OPENVINO=OFF",
    f"-DWITH_OPENVX=OFF",
    f"-DWITH_PLAIDML=OFF",
    f"-DWITH_PNG=OFF",
    f"-DWITH_PROTOBUF=OFF",
    f"-DWITH_QT=OFF",
    f"-DWITH_QUIRC=OFF",
    f"-DWITH_SPNG=OFF",
    f"-DWITH_TBB=OFF",
    f"-DWITH_TIFF=OFF",
    f"-DWITH_TIMVX=OFF",
    f"-DWITH_VTK=OFF",
    f"-DWITH_VULKAN=OFF",
    f"-DWITH_WEBNN=OFF",
    f"-DWITH_WEBP=OFF",
    f"-DWITH_XIMEA=OFF",
    f"-DWITH_ZLIB_NG=OFF",
    f"-Dold-jpeg=OFF",
    f"..",
]

subprocess.run(opencv_cmake_configure_cmd, check=True)
build_command = ['make', f'-j{num_cores}']
subprocess.run(build_command, check=True)
subprocess.run(["make", "install"], check=True)