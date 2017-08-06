protoc -I=./Protolbuf --cpp_out=./MessageStruct ./Protolbuf/*.proto
protoc -I=./Protolbuf/CharLogin/ --cpp_out=./MessageStruct/CharLogin/ ./Protolbuf/CharLogin/*.proto
