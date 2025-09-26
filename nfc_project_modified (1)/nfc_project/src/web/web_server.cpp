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
AppState currentState = STATE_IDLE;
String lastReadUIDForAdd = "";

// --- Log de Leituras ---
const int LOG_MAX_SIZE = 10;
String logUIDs[LOG_MAX_SIZE];
String logTimestamps[LOG_MAX_SIZE];
int logCurrentIndex = 0;

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

void addLog(String uid) {
  logUIDs[logCurrentIndex] = uid;
  logTimestamps[logCurrentIndex] = getFormattedTime();
  logCurrentIndex = (logCurrentIndex + 1) % LOG_MAX_SIZE;
}

// --- Funções do Servidor ---
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
    client.println("<meta http-equiv='refresh' content='10'>");
    client.println("<title>Painel NFC</title>");
    client.println("<style>");
    client.println("* { margin: 0; padding: 0; box-sizing: border-box; }");
    client.println("body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: #f5f7fa; min-height: 100vh; }");
    client.println(".header { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px 0; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }");
    client.println(".header-content { max-width: 1200px; margin: 0 auto; padding: 0 20px; display: flex; justify-content: space-between; align-items: center; }");
    client.println(".header h1 { font-size: 28px; font-weight: 600; }");
    client.println(".logout-btn { background: rgba(255,255,255,0.2); color: white; padding: 8px 16px; border: none; border-radius: 6px; text-decoration: none; font-weight: 500; transition: background 0.3s; }");
    client.println(".logout-btn:hover { background: rgba(255,255,255,0.3); }");
    client.println(".container { max-width: 1200px; margin: 0 auto; padding: 30px 20px; }");
    client.println(".status-card { background: white; border-radius: 12px; padding: 25px; margin-bottom: 30px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); }");
    client.println(".status-indicator { display: flex; align-items: center; justify-content: center; padding: 15px; border-radius: 8px; font-weight: 600; font-size: 18px; }");
    client.println(".status-online { background: linear-gradient(135deg, #10b981 0%, #059669 100%); color: white; }");
    client.println(".status-offline { background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%); color: white; }");
    client.println(".card { background: white; border-radius: 12px; padding: 25px; margin-bottom: 30px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); }");
    client.println(".card h2 { color: #374151; margin-bottom: 20px; font-size: 24px; font-weight: 600; }");
    client.println(".card h3 { color: #374151; margin-bottom: 15px; font-size: 20px; font-weight: 600; }");
    client.println("table { width: 100%; border-collapse: collapse; margin-top: 15px; }");
    client.println("th, td { padding: 12px 15px; text-align: left; border-bottom: 1px solid #e5e7eb; }");
    client.println("th { background: #f9fafb; font-weight: 600; color: #374151; }");
    client.println("tr:hover { background: #f9fafb; }");
    client.println(".btn { display: inline-block; padding: 10px 20px; text-decoration: none; border-radius: 8px; font-weight: 500; transition: all 0.3s; border: none; cursor: pointer; }");
    client.println(".btn-primary { background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%); color: white; }");
    client.println(".btn-primary:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4); }");
    client.println(".btn-danger { background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%); color: white; }");
    client.println(".btn-danger:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(239, 68, 68, 0.4); }");
    client.println(".form-group { margin-bottom: 20px; }");
    client.println(".form-group label { display: block; margin-bottom: 8px; color: #374151; font-weight: 500; }");
    client.println(".form-group input { width: 100%; padding: 12px 15px; border: 2px solid #e5e7eb; border-radius: 8px; font-size: 16px; transition: border-color 0.3s; }");
    client.println(".form-group input:focus { outline: none; border-color: #3b82f6; }");
    client.println(".alert { padding: 15px; border-radius: 8px; margin: 15px 0; font-weight: 500; }");
    client.println(".alert-info { background: #dbeafe; color: #1e40af; border: 1px solid #93c5fd; }");
    client.println(".empty-state { text-align: center; color: #6b7280; padding: 40px 20px; }");
    client.println("@media (max-width: 768px) { .header-content { flex-direction: column; gap: 15px; } .container { padding: 20px 15px; } }");
    client.println("</style>");
    client.println("</head>");
    client.println("<body>");
    
    // Header
    client.println("<div class='header'>");
    client.println("<div class='header-content'>");
    client.println("<h1>Sistema de Controle NFC</h1>");
    client.println("<a href='/logout' class='logout-btn'>Sair</a>");
    client.println("</div>");
    client.println("</div>");
    
    client.println("<div class='container'>");
    
    // Status do Sensor
    client.println("<div class='status-card'>");
    client.println("<h2>Status do Sistema</h2>");
    if(sensorOnline) { 
        client.println("<div class='status-indicator status-online'>SENSOR ONLINE</div>"); 
    } else { 
        client.println("<div class='status-indicator status-offline'>SENSOR OFFLINE</div>"); 
    }
    client.println("<p style='margin-top: 15px; color: #6b7280;'>Ultima atualizacao: " + getFormattedTime() + "</p>");
    client.println("</div>");

    // Log de Leituras
    client.println("<div class='card'>");
    client.println("<h2>Ultimos 10 Registros</h2>");
    bool hasLogs = false;
    for(int i=0; i<LOG_MAX_SIZE; i++){
        if(logUIDs[i] != "" && logUIDs[i] != NULL) {
            hasLogs = true;
            break;
        }
    }
    
    if(hasLogs) {
        client.println("<table>");
        client.println("<tr><th>Data/Hora</th><th>UID do Cartao</th></tr>");
        for(int i=0; i<LOG_MAX_SIZE; i++){
            if(logUIDs[i] != "" && logUIDs[i] != NULL) {
                client.println("<tr><td>" + logTimestamps[i] + "</td><td><code>" + logUIDs[i] + "</code></td></tr>");
            }
        }
        client.println("</table>");
    } else {
        client.println("<div class='empty-state'>Nenhum registro encontrado</div>");
    }
    client.println("</div>");

    // Cadastro de Usuários
    client.println("<div class='card'>");
    client.println("<h2>Gerenciamento de Cartoes</h2>");
    client.println("<a href='/add' class='btn btn-primary'>Cadastrar Novo Cartao</a>");
    
    if(currentState == STATE_WAITING_FOR_CARD_TO_ADD) {
        client.println("<div class='alert alert-info'>Aproxime o cartao ou celular para cadastrar...</div>");
    }
    
    if(lastReadUIDForAdd != "") {
        client.println("<div style='background: #f0f9ff; padding: 20px; border-radius: 8px; margin: 20px 0; border: 1px solid #0ea5e9;'>");
        client.println("<h4 style='color: #0369a1; margin-bottom: 15px;'>Cartao Detectado!</h4>");
        client.println("<form action='/save' method='post'>");
        client.println("<p><strong>UID Lido:</strong> <code>" + lastReadUIDForAdd + "</code></p>");
        client.println("<input type='hidden' name='uid' value='" + lastReadUIDForAdd + "'>");
        client.println("<div class='form-group'>");
        client.println("<label for='name'>Nome da Pessoa:</label>");
        client.println("<input type='text' id='name' name='name' placeholder='Ex: Joao Silva' required>");
        client.println("</div>");
        client.println("<button type='submit' class='btn btn-primary'>Salvar Cartao</button>");
        client.println("</form>");
        client.println("</div>");
    }

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
    
    client.println("</div>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
}

void handleClient() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("Novo cliente conectado!");
  String request_line = client.readStringUntil('\r');
  
  if (request_line.indexOf("POST /login") != -1) {
    // Lógica melhorada para ler o corpo da requisição POST
    String line;
    while(client.connected() && client.available()){
      line = client.readStringUntil('\n');
      if(line.length() == 1 && line[0] == '\r'){ // Encontra a linha em branco que separa os cabeçalhos do corpo
        break;
      }
    }
    String body = client.readStringUntil('\n'); // A próxima linha é o corpo

    Serial.println("--- DADOS DE LOGIN RECEBIDOS (Metodo Robusto) ---");
    Serial.println("BODY: [" + body + "]");
    Serial.println("------------------------------------------------");

    if (body.indexOf("username=" + String(adminUser)) != -1 && body.indexOf("password=" + String(adminPass)) != -1) {
      isLoggedIn = true;
      serveMainPage(client);
    } else {
      serveLoginPage(client, true);
    }

  } else if (request_line.indexOf("GET /delete") != -1) {
    int idIndex = request_line.indexOf("id=");
    if (idIndex != -1) {
      int id = request_line.substring(idIndex + 3).toInt();
      deleteUser(id);
    }
    serveMainPage(client);

  } else if (request_line.indexOf("GET /add") != -1) {
    currentState = STATE_WAITING_FOR_CARD_TO_ADD;
    lastReadUIDForAdd = "";
    serveMainPage(client);

  } else if (request_line.indexOf("GET /logout") != -1) {
    isLoggedIn = false;
    serveLoginPage(client, false);

  } else if (request_line.indexOf("POST /save") != -1) {
    String line;
    while(client.connected() && client.available()){
      line = client.readStringUntil('\n');
      if(line.length() == 1 && line[0] == '\r'){
        break;
      }
    }
    String body = client.readStringUntil('\n');
    
    Serial.println("--- DADOS DE SAVE RECEBIDOS ---");
    Serial.println("BODY: [" + body + "]");
    Serial.println("-------------------------------");
    
    String uid = body.substring(body.indexOf("uid=") + 4, body.indexOf("&"));
    String name = body.substring(body.indexOf("name=") + 5);
    name.replace("+", " ");
    name.replace("%20", " ");
    
    Serial.println("UID: " + uid);
    Serial.println("Nome: " + name);
    
    bool success = addUser(uid, name);
    Serial.println("Resultado do addUser: " + String(success));
    
    lastReadUIDForAdd = "";
    currentState = STATE_IDLE;
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
  Serial.println("Cliente desconectado.");
}
