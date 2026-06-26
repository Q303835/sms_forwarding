<?php
// 允许跨域请求
header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Headers: *");

// 如果是用浏览器直接打开（GET 请求），显示一段友好的提示
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    echo "<h2 style='color: green;'>✅ 代理脚本运行正常！</h2>";
    echo "<p>请在 ESP32 代码中使用 POST 方法向此地址发送通知数据。</p>";
    exit();
}

// 提取 ESP32 发来的目标运营商服务器
$targetHost = $_SERVER['HTTP_X_TARGET_HOST'] ?? '';
if (!$targetHost) {
    http_response_code(400);
    exit("Missing X-Target-Host header");
}

$targetUrl = "https://" . $targetHost . "/gsma/rsp2/es9plus/handleNotification";
$body = file_get_contents('php://input');

$ch = curl_init($targetUrl);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, $body);
curl_setopt($ch, CURLOPT_HTTPHEADER, [
    'Content-Type: application/json',
    'X-Admin-Protocol: gsma/rsp/v2.2.0',
    'User-Agent: gsma-rsp-lpad'
]);

// 🔥 强制忽略电信级证书的校验！
curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
curl_setopt($ch, CURLOPT_TIMEOUT, 15);

$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);

if (curl_errno($ch)) {
    http_response_code(502);
    echo "cURL Error: " . curl_error($ch);
} else {
    http_response_code($httpCode);
    echo $response;
}
curl_close($ch);
?>