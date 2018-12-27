'use strict';

const os = require("os");
const express = require('express');
var cluster = require("cluster");

const PORT = 8080;
const HOST = '0.0.0.0';

function start_server(id) {
    const app = express();
    app.get('/', (req, res) => {
        res.send('OK');
    });
    app.listen(PORT, HOST);
    console.log(`Running on http://${HOST}:${PORT}`);
}
function main() {
    if (cluster.isMaster) {
        for (let i = 1; i <= os.cpus().length; i++) {
            cluster.fork({"id": i});
        }
    } else {
        start_server(cluster.worker.id);
    }
}

main();
