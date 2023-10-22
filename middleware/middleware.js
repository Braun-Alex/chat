const express = require('express');
const { Server: WebSocketServer } = require('ws');
const net = require('net');

const WEB_PORT = 3000;
const MAIN_SERVER_PORT = 8080;

const app = express();
const server = app.listen(WEB_PORT, () => {
    console.log(`Middleware server is listening on http://localhost:${WEB_PORT}`);
});

const wss = new WebSocketServer({ server });

wss.on('connection', (ws) => {
    console.log('Client connected');

    const tcpClient = new net.Socket();

    tcpClient.connect(MAIN_SERVER_PORT, 'localhost', () => {
        console.log('Connected to main server');
    });

    ws.on('message', (message) => {
        tcpClient.write(message);
    });

    tcpClient.on('data', (data) => {
        ws.send(data.toString());
    });

    ws.on('close', () => {
        tcpClient.end();
        console.log('Client disconnected');
    });
});
