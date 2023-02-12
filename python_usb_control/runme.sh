rm -Rf pb_c
mkdir -p pb_c
mkdir -p pb_c/defs
# Gen C++ side
nanopb_generator.py  -S .cpp -I pb/defs -D pb_c/defs -T defines.proto
nanopb_generator.py  -S .cpp -I pb -D pb_c -T messaging.proto
# Gen python side
 #protoc -I=pb/defs  --python_out=tmp_py defines.proto
 protoc -I=pb  --python_out=py messaging.proto defs/defines.proto
