#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define RELAY_PIN 5  // 定义继电器控制引脚 (GPIO5，对应 D1)

// Wi-Fi 配置
const char* ssid = "wifi名称";  // 在这里配置将要连接的WIFI名称
const char* password = "wifi密码";  // 在这里配置将要连接的WIFI密码

// 静态 IP 配置
IPAddress local_IP(192, 168, 1, 150);  // 设定ESP8266的IP地址
IPAddress gateway(192, 168, 1, 1);  // 设定默认网关
IPAddress subnet(255, 255, 255, 0);  // 设定子网掩码
IPAddress primaryDNS(192, 168, 1, 1);  // 设定首选DNS服务器

// 创建 Web 服务器
ESP8266WebServer server(80);

// 生成 HTML 网页
String generateHTML() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>电脑控制</title>";
  html += "<style>";
  html += "body{display:flex;flex-direction:column;align-items:center;justify-content:center;min-height:90vh;margin:0;font-family:sans-serif;}";
  html += "h1{font-size:22px;margin-bottom:20px;}";
  html += ".btn{width:80%;max-width:300px;padding:15px;font-size:18px;margin:10px 0;cursor:pointer;}";
  html += "#status{margin-top:20px;font-size:16px;color:#333;}";
  html += "</style></head><body>";
  html += "<h1>电脑控制页面</h1>";
  html += "<button class=\"btn\" onclick=\"sendCmd('/power')\">开机或关机</button>";
  html += "<button class=\"btn\" onclick=\"sendCmd('/shutdown')\">强制关机</button>";
  html += "<div id=\"status\">状态：就绪</div>";

  // JS 逻辑：异步发送 POST 请求并更新状态
  html += "<script>";
  html += "function sendCmd(path){";
  html += "  var st = document.getElementById('status');";
  html += "  st.innerText = '状态：动作中...';";
  html += "  var btns = document.querySelectorAll('.btn');";
  html += "  btns.forEach(b => b.disabled = true);";
  html += "  fetch(path, {method:'POST'}).then(res => res.text()).then(() => {";
  html += "    st.innerText = '状态：动作完成！';";
  html += "    setTimeout(() => { st.innerText = '状态：就绪'; btns.forEach(b => b.disabled = false); }, 3000);";
  html += "  }).catch(() => {";
  html += "    st.innerText = '状态：动作失败，请重试！';";
  html += "    btns.forEach(b => b.disabled = false);";
  html += "  });";
  html += "}";
  html += "</script>";

  html += "</body></html>";
  return html;
}

// 处理主页请求
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", generateHTML());
}

// 处理开机请求
void handlePower() {
  digitalWrite(RELAY_PIN, HIGH);  // 启动继电器
  delay(1000);                    // 模拟按下开机键 1 秒
  digitalWrite(RELAY_PIN, LOW);   // 释放继电器
  server.send(200, "text/plain", "OK");
}

// 处理强制关机请求
void handleShutdown() {
  digitalWrite(RELAY_PIN, HIGH);  // 启动继电器
  delay(5000);                    // 模拟按下电源键 5 秒
  digitalWrite(RELAY_PIN, LOW);   // 释放继电器
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

// 配置静态 IP
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
  Serial.println("静态 IP 配置失败！");
}

WiFi.begin(ssid, password);
Serial.print("正在连接 WiFi");

unsigned long startAttemptTime = millis();
const unsigned long connectTimeout = 20000;  // 先尝试 20 秒

while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < connectTimeout) {
  delay(500);
  Serial.print(".");
}

if (WiFi.status() != WL_CONNECTED) {
  Serial.println("\nWiFi 连接失败！等待 2 分钟后重启...");
  delay(120000);  // 等待 2 分钟
  ESP.restart();
} else {
  Serial.println("\nWiFi 已连接！");
  Serial.print("设备 IP 地址: ");
  Serial.println(WiFi.localIP());
}

  // 设置服务器路由
  server.on("/", handleRoot);
  server.on("/power", HTTP_POST, handlePower);
  server.on("/shutdown", HTTP_POST, handleShutdown);

  // 启动服务器
  server.begin();
  Serial.println("Web 服务器已启动！");
}

void loop() {
  server.handleClient();
}