rm tmp/*/*
rm tmp/*
rm tmp_py/*
mkdir -p tmp/defs
# Gen C++ side
nanopb_generator.py  -S .cpp -I pb/defs -D tmp/defs -T defines.proto
nanopb_generator.py  -S .cpp -I pb -D tmp -T messaging.proto
# Gen python side
 protoc -I=pb/defs  --python_out=tmp_py defines.proto
 protoc -I=pb  --python_out=tmp_py messaging.proto
