#!/bin/sh

set -e
set -x

curl -X PUT "http://homeapps1:8298/_template/firewall" -H "Content-Type: application/json" -d @elasticsearch-template.json
