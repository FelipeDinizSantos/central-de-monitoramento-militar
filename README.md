# Sistema de Monitoramento com ESP32-CAM + Laravel

## Sobre o Projeto

Este projeto consiste em um sistema de monitoramento em tempo real para armária militar, utilizando:

* ESP32-CAM
* MQTT
* API Laravel
* Interface Web com Blade + Bootstrap + jQuery

O objetivo do sistema é:

1. Detectar movimento com base em proximidade via MQTT
2. Capturar imagens automaticamente utilizando a ESP32-CAM como subscribe
3. Enviar as imagens para uma API Laravel
4. Armazenar os registros no banco de dados
5. Exibir as capturas em um painel
   
---

# Arquitetura do Projeto

```text
Arduino/MQTT
     ↓
ESP32-CAM
     ↓
HTTP POST
     ↓
Laravel API
     ↓
Banco de Dados
     ↓
Painel Web Blade
```

---

# Tecnologias Utilizadas

## Firmware ESP32

* C++
* Arduino IDE
* WiFi.h
* PubSubClient
* HTTPClient
* esp_camera

## Backend

* Laravel
* MariaDB 

## Frontend

* Blade
* Bootstrap
* jQuery

---

# Funcionalidades

## Arduino / Sensor 

* Leitura de sensores
* Publicação MQTT
* Envio de eventos em tempo real
* Comunicação com broker MQTT

## ESP32-CAM

* Conexão Wi-Fi
* Sincronização de horário via NTP
* Inscrição em tópico MQTT
* Captura automática de imagens
* Upload HTTP para API Laravel
* Controle de cooldown
* Correção de buffer antigo da câmera

---

## API Laravel

* Recebimento de imagens
* Armazenamento local
* Registro em banco de dados
* Consulta dos registros
* Filtros por:

  * período
  * horário

---

## Painel Web

* Visualização das imagens
* Modal para ampliação
* Filtragem

---

# Estrutura do Banco

## Tabela `registros_acesso`

| Campo      | Tipo      |
| ---------- | --------- |
| id         | bigint    |
| device_id  | varchar   |
| mensagem   | text      |
| imagem     | varchar   |
| created_at | timestamp |
| updated_at | timestamp |

---

# Como Rodar o Projeto

# 1. Clonar o Projeto

```bash
git clone SEU_REPOSITORIO
```

---

# 2. Instalar Dependências

```bash
composer install
```

---

# 3. Configurar `.env`

Configure:

```env
DB_DATABASE=esp32_api
DB_USERNAME=root
DB_PASSWORD=
```

---

# 4. Rodar Migration

```bash
php artisan migrate
```

---

# 5. Criar Link do Storage

```bash
php artisan storage:link
```

---

# 6. Subir o Laravel

IMPORTANTE:

```bash
php artisan serve --host=0.0.0.0 --port=8000
```

O `0.0.0.0` é necessário para permitir acesso da ESP32 pela rede local.

---

# 7. Configurar IP da API no ESP32

No firmware:

```cpp
const char* API_URL =
"http://SEU_IP_LOCAL:8000/api/registros-acesso";
```

Exemplo:

```cpp
const char* API_URL =
"http://192.168.15.6:8000/api/registros-acesso";
```

---

# 8. Upload do Firmware

Na Arduino IDE:

* selecione a placa ESP32-CAM AI Thinker
* configure a porta COM
* faça upload do código

---

# 9. Estrutura Esperada da Rede

IMPORTANTE:

O computador e a ESP32 devem estar:

* no mesmo Wi-Fi
* na mesma subrede

Evite:

* hotspot do Windows
* VPN
* redes isoladas

---

Broker utilizado:

```text
broker.hivemq.com
```

---

Projeto com fins acadêmicos. 
