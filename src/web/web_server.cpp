#include "web_server.h"
#include "../hw/hardware_control.h"
#include "../main/data_manager.h"
#include "time.h"

// --- Configurações de Rede e Login ---
const char* ssid = "PRIME ESCRITORIO";     // <<-- MUDE AQUI
const char* password = "hazeng4N"; // <<-- MUDE AQUI
const char* adminUser = "admin";
const char* adminPass = "5298";

// --- Variáveis de Estado ---
WiFiServer server(80);
bool isLoggedIn = false;

// --- Funções de Tempo ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600; // Fuso de São Paulo (UTC-3)
const int daylightOffset_sec = 0;

String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "N/A";
  }
  char timeString[20];
  strftime(timeString, sizeof(timeString), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(timeString);
}

void addAccessLog(String uid, bool authorized) {
  uid.toCharArray(accessLogs[logCurrentIndex].uid, 30);
  getFormattedTime().toCharArray(accessLogs[logCurrentIndex].timestamp, 20);
  String status = authorized ? "Autorizado" : "Nao Autorizado";
  status.toCharArray(accessLogs[logCurrentIndex].status, 20);
  logCurrentIndex = (logCurrentIndex + 1) % LOG_MAX_SIZE;
  saveLogsToPrefs();  // Salva imediatamente
}

bool isAuthorized(String uid) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (String(authorizedUsers[i].uid) == uid) {
      return true;
    }
  }
  return false;
}

// Função para decodificar URL (substitui %3A por :, etc.)
String urlDecode(String str) {
  String decoded = "";
  for (unsigned int i = 0; i < str.length(); i++) {
    if (str[i] == '%') {
      if (i + 2 < str.length()) {
        String hex = str.substring(i + 1, i + 3);
        char decodedChar = (char) strtol(hex.c_str(), NULL, 16);
        decoded += decodedChar;
        i += 2;
      }
    } else if (str[i] == '+') {
      decoded += ' ';
    } else {
      decoded += str[i];
    }
  }
  return decoded;
}

void initWebServer() {
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: http://");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.begin();
}

void serveLoginPage(WiFiClient client, bool showError) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html; charset=UTF-8");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html lang='pt-BR'>");
  client.println("<head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>Login - Sistema NFC</title>");
  client.println("<style>");
  client.println("* { margin: 0; padding: 0; box-sizing: border-box; }");
  client.println("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; align-items: center; justify-content: center; }");
  client.println(".login-container { background: white; padding: 40px; border-radius: 15px; box-shadow: 0 15px 35px rgba(0,0,0,0.1); width: 100%; max-width: 400px; }");
  client.println(".login-header { text-align: center; margin-bottom: 30px; }");
  client.println(".login-header h2 { color: #333; font-size: 28px; margin-bottom: 10px; }");
  client.println(".login-header p { color: #666; font-size: 14px; }");
  client.println(".form-group { margin-bottom: 20px; }");
  client.println(".form-group label { display: block; margin-bottom: 8px; color: #555; font-weight: 500; }");
  client.println(".form-group input { width: 100%; padding: 12px 15px; border: 2px solid #e1e5e9; border-radius: 8px; font-size: 16px; transition: border-color 0.3s; }");
  client.println(".form-group input:focus { outline: none; border-color: #667eea; }");
  client.println(".login-btn { width: 100%; padding: 12px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 8px; font-size: 16px; font-weight: 600; cursor: pointer; transition: transform 0.2s; }");
  client.println(".login-btn:hover { transform: translateY(-2px); }");
  client.println(".error { background: #fee; border: 1px solid #fcc; color: #c33; padding: 10px; border-radius: 5px; margin-top: 15px; text-align: center; }");
  client.println("@media (max-width: 480px) { .login-container { margin: 20px; padding: 30px 20px; } }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<div class='login-container'>");
  client.println("<div class='login-header'>");
  client.println("<h2>Sistema NFC</h2>");
  client.println("<p>Acesso ao painel de controle</p>");
  client.println("</div>");
  client.println("<form action='/login' method='post'>");
  client.println("<div class='form-group'>");
  client.println("<label for='username'>Usuario</label>");
  client.println("<input type='text' id='username' name='username' required>");
  client.println("</div>");
  client.println("<div class='form-group'>");
  client.println("<label for='password'>Senha</label>");
  client.println("<input type='password' id='password' name='password' required>");
  client.println("</div>");
  client.println("<button type='submit' class='login-btn'>Entrar</button>");
  if (showError) {
    client.println("<div class='error'>Usuario ou senha invalidos!</div>");
  }
  client.println("</form>");
  client.println("</div>");
  client.println("</body>");
  client.println("</html>");
}

void serveMainPage(WiFiClient client) {
    // Cabeçalhos HTTP
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();

    // Início do HTML
    client.println("<!DOCTYPE html>");
    client.println("<html lang='pt-BR'>");
    client.println("<head>");
    client.println("<meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>Painel NFC</title>");
    client.println("<style>");
    client.println("* { margin: 0; padding: 0; box-sizing: border-box; }");
    client.println("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f5f7fa; min-height: 100vh; }");
    client.println(".header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px 0; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }");
    client.println(".header-content { max-width: 1200px; margin: 0 auto; padding: 0 20px; display: flex; justify-content: space-between; align-items: center; }");
    client.println(".header h1 { font-size: 24px; }");
    client.println(".logout-btn { background: white; color: #667eea; padding: 8px 16px; border-radius: 20px; text-decoration: none; font-weight: 600; transition: background 0.3s; }");
    client.println(".logout-btn:hover { background: #f0f0f0; }");
    client.println(".container { max-width: 1200px; margin: 40px auto; padding: 0 20px; }");
    client.println(".card { background: white; padding: 30px; border-radius: 15px; box-shadow: 0 5px 15px rgba(0,0,0,0.05); margin-bottom: 40px; }");
    client.println("h2 { color: #333; margin-bottom: 20px; font-size: 24px; }");
    client.println("h3 { color: #444; margin: 30px 0 15px; font-size: 20px; }");
    client.println("table { width: 100%; border-collapse: collapse; margin-top: 20px; }");
    client.println("th, td { padding: 15px; text-align: left; border-bottom: 1px solid #eee; }");
    client.println("th { background: #f8f9fa; font-weight: 600; color: #555; }");
    client.println("code { background: #f4f4f5; padding: 4px 8px; border-radius: 4px; font-family: monospace; }");
    client.println(".btn { display: inline-block; padding: 10px 20px; border-radius: 8px; text-decoration: none; font-weight: 600; transition: transform 0.2s; cursor: pointer; }");
    client.println(".btn-primary { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; }");
    client.println(".btn-primary:hover { transform: translateY(-2px); }");
    client.println(".btn-danger { background: #ef4444; color: white; }");
    client.println(".btn-danger:hover { transform: translateY(-2px); background: #dc2626; }");
    client.println(".form-group { margin-bottom: 20px; }");
    client.println(".form-group label { display: block; margin-bottom: 8px; color: #555; font-weight: 500; }");
    client.println(".form-group input { width: 100%; padding: 12px 15px; border: 2px solid #e1e5e9; border-radius: 8px; font-size: 16px; transition: border-color 0.3s; }");
    client.println(".form-group input:focus { outline: none; border-color: #667eea; }");
    client.println(".empty-state { text-align: center; color: #888; font-style: italic; padding: 20px; background: #f9fafb; border-radius: 8px; }");
    client.println("@media (max-width: 768px) { table { font-size: 14px; } th, td { padding: 10px; } .container { padding: 0 15px; } }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    client.println("<header class='header'>");
    client.println("<div class='header-content'>");
    client.println("<h1>Painel de Controle NFC</h1>");
    client.println("<a href='/logout' class='logout-btn'>Sair</a>");
    client.println("</div>");
    client.println("</header>");
    client.println("<div class='container'>");

    // Cadastro de Usuários
    client.println("<div class='card'>");
    client.println("<h2>Gerenciamento de Cartoes</h2>");

    // Formulário para adicionar manualmente
    client.println("<h3>Adicionar Novo Cartao</h3>");
    client.println("<form action='/save' method='post'>");
    client.println("<div class='form-group'>");
    client.println("<label for='uid'>UID do Cartao (ex: 04:AB:CD:EF:12:34:56):</label>");
    client.println("<input type='text' id='uid' name='uid' placeholder='Digite o UID' required>");
    client.println("</div>");
    client.println("<div class='form-group'>");
    client.println("<label for='name'>Nome da Pessoa:</label>");
    client.println("<input type='text' id='name' name='name' placeholder='Ex: Joao Silva' required>");
    client.println("</div>");
    client.println("<button type='submit' class='btn btn-primary'>Adicionar Cartao</button>");
    client.println("</form>");

    // Lista de UIDs Cadastrados
    client.println("<h3>Cartoes Autorizados</h3>");
    bool hasUsers = false;
    for(int i=0; i<MAX_USERS; i++){
        if(strlen(authorizedUsers[i].uid) > 0){
            hasUsers = true;
            break;
        }
    }
    
    if(hasUsers) {
        client.println("<table>");
        client.println("<tr><th>Nome</th><th>UID do Cartao</th><th>Acoes</th></tr>");
        for(int i=0; i<MAX_USERS; i++){
            if(strlen(authorizedUsers[i].uid) > 0){
                client.println("<tr>");
                client.println("<td><strong>" + String(authorizedUsers[i].name) + "</strong></td>");
                client.println("<td><code>" + String(authorizedUsers[i].uid) + "</code></td>");
                client.println("<td><a href='/delete?id=" + String(i) + "' class='btn btn-danger' onclick='return confirm(\"Tem certeza que deseja deletar este cartao?\")'>Deletar</a></td>");
                client.println("</tr>");
            }
        }
        client.println("</table>");
    } else {
        client.println("<div class='empty-state'>Nenhum cartao cadastrado</div>");
    }

    // Seção de Últimos Acessos
    client.println("<h3>Ultimos Acessos (ultimos 30)</h3>");
    client.println("<table>");
    client.println("<tr><th>Data/Hora</th><th>UID</th><th>Status</th></tr>");
    // Exibe em ordem reversa (mais recente primeiro)
    for (int i = 0; i < LOG_MAX_SIZE; i++) {
      int idx = (logCurrentIndex - 1 - i + LOG_MAX_SIZE) % LOG_MAX_SIZE;
      if (strlen(accessLogs[idx].uid) > 0) {  // So exibe se nao vazio
        client.println("<tr>");
        client.println("<td>" + String(accessLogs[idx].timestamp) + "</td>");
        client.println("<td><code>" + String(accessLogs[idx].uid) + "</code></td>");
        client.println("<td>" + String(accessLogs[idx].status) + "</td>");
        client.println("</tr>");
      }
    }
    client.println("</table>");
    
    client.println("</div>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
}

void handleClient() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Novo cliente conectado!");

  // Lê a requisição linha por linha
  String req = "";
  String header = "";
  int contentLength = 0;
  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) {  // Linha vazia separa headers do body
        break;
      }
      header += line + "\n";

      // Parse Content-Length se for POST
      if (line.startsWith("Content-Length: ")) {
        contentLength = line.substring(16).toInt();
      }

      if (req == "") {  // Primeira linha é a request line
        req = line;
      }
    }
  }

  // Lê o body exatamente com base em Content-Length
  String body = "";
  if (contentLength > 0) {
    unsigned long timeout = millis() + 5000;  // Timeout de 5s
    while (body.length() < contentLength && millis() < timeout) {
      if (client.available()) {
        body += (char)client.read();
      }
    }
  }

  Serial.println("Request line: " + req);
  Serial.println("Headers: ");
  Serial.println(header);
  Serial.println("Body: " + body);

  // Agora processa baseado na req
  if (req.indexOf("POST /login") != -1) {
    Serial.println("--- DADOS DE LOGIN RECEBIDOS ---");
    Serial.println("BODY: [" + body + "]");
    Serial.println("------------------------------------------------");

    // Parse username e password com decode
    int userStart = body.indexOf("username=") + 9;
    int userEnd = body.indexOf("&", userStart);
    String username = urlDecode(body.substring(userStart, userEnd));

    int passStart = body.indexOf("password=") + 9;
    String passwordStr = urlDecode(body.substring(passStart));

    if (username == adminUser && passwordStr == adminPass) {
      isLoggedIn = true;
      serveMainPage(client);
    } else {
      serveLoginPage(client, true);
    }

  } else if (req.indexOf("GET /delete") != -1) {
    int idIndex = req.indexOf("id=");
    if (idIndex != -1) {
      int id = req.substring(idIndex + 3, req.indexOf(" ", idIndex)).toInt();
      deleteUser(id);
    }
    serveMainPage(client);

  } else if (req.indexOf("GET /logout") != -1) {
    isLoggedIn = false;
    serveLoginPage(client, false);

  } else if (req.indexOf("POST /save") != -1) {
    Serial.println("--- DADOS DE SAVE RECEBIDOS ---");
    Serial.println("BODY: [" + body + "]");
    Serial.println("-------------------------------");

    // Parse uid e name com decode
    int uidStart = body.indexOf("uid=") + 4;
    int uidEnd = body.indexOf("&", uidStart);
    String uid = urlDecode(body.substring(uidStart, uidEnd));

    int nameStart = body.indexOf("name=") + 5;
    String name = urlDecode(body.substring(nameStart));

    Serial.println("UID decodificado: " + uid);
    Serial.println("Nome decodificado: " + name);

    bool success = addUser(uid, name);
    Serial.println("Resultado do addUser: " + String(success));

    serveMainPage(client);

  } else {
    if (isLoggedIn) {
      serveMainPage(client);
    } else {
      serveLoginPage(client, false);
    }
  }

  client.flush();
  delay(10);
  client.stop();
  Serial.println("Cliente desconectado.");
}