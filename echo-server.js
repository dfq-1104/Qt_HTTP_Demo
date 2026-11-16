const http = require('node:http');
const address = 'http://127.0.0.1:8080';

function sendJson(response, status, data) {
    response.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8' });
    response.end(JSON.stringify(data, null, 2));
}

http.createServer((request, response) => {
    const url = new URL(request.url, address);
    if (request.method === 'GET' && url.pathname === '/slow') {
        const timer = setTimeout(() => sendJson(response, 200, { delayed: true }), 12000);
        response.once('close', () => clearTimeout(timer));
        return;
    }
    if (url.pathname !== '/echo') {
        sendJson(response, 404, { error: '路径不存在', path: url.pathname });
        return;
    }
    if (request.method === 'GET') {
        sendJson(response, 200, { method: 'GET', query: Object.fromEntries(url.searchParams) });
        return;
    }
    if (request.method === 'POST') {
        let body = '';
        request.setEncoding('utf8');
        request.on('data', chunk => { body += chunk; });
        request.on('end', () => {
            sendJson(response, 200, { method: 'POST', contentType: request.headers['content-type'], body });
        });
        request.on('error', () => response.destroy());
        return;
    }
    response.setHeader('Allow', 'GET, POST');
    sendJson(response, 405, { error: '仅支持 GET 和 POST' });
}).listen(8080, '127.0.0.1', () => console.log(`回显服务已启动 ${address}`));
