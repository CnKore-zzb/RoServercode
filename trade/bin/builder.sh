#!/bin/sh

# BASE_DIR  交易所php程序目录
# PROTO_DIR Proto协议文件目录

BASE_DIR=$(cd `dirname $0`; pwd)/../
PROTO_DIR=${BASE_DIR}../Proto/

rm -rf ${BASE_DIR}/proto/RO/
protoc --php_out='options%5Bmultifile%5D=1&options%5Bnamespace%5D=RO\Cmd:'${BASE_DIR}'proto/' \
--plugin=protoc-gen-php=${BASE_DIR}'/proto/protobuf-php/protoc-gen-php.php' \
--proto_path ${PROTO_DIR} \
${PROTO_DIR}SceneTrade.proto \
${PROTO_DIR}SceneItem.proto \
${PROTO_DIR}SocialCmd.proto \
${PROTO_DIR}SceneUser2.proto \
${PROTO_DIR}SessionCmd.proto \
${PROTO_DIR}LogCmd.proto \
${PROTO_DIR}StatCmd.proto \
${PROTO_DIR}SystemCmd.proto


cd ${BASE_DIR}
composer dump-autoload --optimize
