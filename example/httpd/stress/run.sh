#!/bin/sh
docker run -it --rm \
  --network="host" \
  -v "`pwd`:/stress:ro" \
  -e SERVER=http://127.0.0.1:8080 \
  loadimpact/k6 run --vus 100 --duration 30s /stress/stress.js
