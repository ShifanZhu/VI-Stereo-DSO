#!/bin/bash
#usage: python xxx.py file_name
dataname="MH_01_easy"
#dataname="MH_02_easy"
#dataname="MH_03_medium"
#dataname="MH_04_difficult"
#dataname="MH_05_difficult"
#dataname="V2_03_difficult"
#dataname="V2_01_easy"
#dataname="V2_02_medium"
#dataname="V1_02_medium"
#dataname="V1_01_easy"
#dataname="V1_03_difficult"
    # run dso
     ./build/bin/dso_ros \
 	  calib=/home/zh/code/VI-Stereo-DSO/calib/euroc/cam0.txt \
	  imu_info=/home/zh/code/VI-Stereo-DSO/calib/euroc/IMU_info.txt \
	  groundtruth=/home/zh/data/bag/vins/${dataname}/data.csv \
 	  preset=0 mode=1 \
	  quiet=0 nomt=1 \
          savefile_tail=nt_${dataname}\
	  use_stereo=0\
	  imu_weight=6 imu_weight_tracker=0.6 stereo_weight=0



#     ./build/bin/dso_dataset \
# 	  files0=/home/zh/data/bag/vins/${dataname}/${dataname}/mav0/cam0/data \
# 	  calib0=/home/zh/code/VI-Stereo-DSO/calib/euroc/cam0.txt \
#	  imu_info=/home/zh/code/VI-Stereo-DSO/calib/euroc/IMU_info.txt \
#	  groundtruth=/home/zh/data/bag/vins/${dataname}/data.csv \
#	  imudata=/home/zh/data/bag/vins/${dataname}/${dataname}/mav0/imu0/data.csv\
#	  pic_timestamp=/home/zh/data/bag/vins/${dataname}/${dataname}/mav0/cam0/data.csv \
#	  preset=0 mode=1 \
#	  quiet=1 nomt=1 \
#          savefile_tail=nt_${dataname}\
#	  use_stereo=0\
#	  imu_weight=6 imu_weight_tracker=0.6 stereo_weight=0
