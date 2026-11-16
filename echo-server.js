const http = require('node:http');
const address = 'http://127.0.0.1:8080';

// 将 JavaScript 对象序列化为 JSON 正文，和 HTTP 状态码、编码声明一起发给客户端。
function sendJson(response, status, data) {
    response.writeHead(status, { 'Content-Type': 'application/json; charset=utf-8' });
    response.end(JSON.stringify(data, null, 2));
}

// request 是收到的 HTTP 请求，response 用来写回这次请求的响应。
http.createServer((request, response) => {
    // request.url 通常形如 /echo?name=Qt。用本机地址作基准，拆出路径和查询参数。
    const url = new URL(request.url, address);
    if (request.method === 'GET' && url.pathname === '/slow') {
        // 服务端故意比客户端的 10 秒超时多等一会，连接关闭时取消延迟回复。
        const timer = setTimeout(() => sendJson(response, 200, { delayed: true }), 12000);
        response.once('close', () => clearTimeout(timer));
        return;
    }
    if (url.pathname !== '/echo') {
        sendJson(response, 404, { error: '路径不存在', path: url.pathname });
        return;
    }
    if (request.method === 'GET') {
        // URLSearchParams 读取并解码查询参数，Object.fromEntries 把它们变成回显对象。
        sendJson(response, 200, { method: 'GET', query: Object.fromEntries(url.searchParams) });
        return;
    }
    if (request.method === 'POST') {
        let body = '';
        // 请求体是流，可能分多次到达。由流统一进行 UTF-8 解码，避免分块截断中文字符。
        request.setEncoding('utf8');
        request.on('data', chunk => { body += chunk; });
        // end 表示正文已经收齐。body 保留为字符串，外层 JSON 序列化时会转义其中的引号。
        request.on('end', () => {
            sendJson(response, 200, { method: 'POST', contentType: request.headers['content-type'], body });
        });
        request.on('error', () => response.destroy());
        return;
    }
    response.setHeader('Allow', 'GET, POST');
    sendJson(response, 405, { error: '仅支持 GET 和 POST' });
}).listen(8080, '127.0.0.1', () => console.log(`回显服务已启动 ${address}`));
