<img src="https://github.com/marceloams/smart-device/blob/master/assets/readme_header.png">

# Smart Device

## Aplicação 

> Sensores modulares conectados à internet, com banco de dados e aplicativo mobile como interface do usuário.

---

## Características

> **Microcontrolador:** ESP8266

> **IDE:** Arduino IDE 1.8.13

> **Linguagem de programação:** C++

> **Sensores:** DHT11 e sensor de presença PIR 

---

## Organização do diretório

- <a href="https://github.com/marceloams/smart-device/smart-device" target="_blank">Assets</a>
- <a href="https://github.com/marceloams/smart-device/smart-device" target="_blank">Projeto</a>

---

## Como utilizar - passo a passo

### Requisitos

 - Possuir os componentes citados à cima bem como o software da <a href="https://www.arduino.cc/en/software" target="_blank">IDE Arduino</a>. 

### Passo 1

  - 👯 Clonar o repositório.

### Passo 2

- Passe o código para o microcontrolador e irá começar como um ponto de acesso, com as seguintes características:

<img src="https://github.com/marceloams/smart-device/blob/master/assets/ponto_de_acesso.png">

### Passo 3

- Após conectar, abra o navegador e acesse o endereço 192.168.4.1 e irá abrir a tela de configuração do dispositivo:

<img src="https://github.com/marceloams/smart-device/blob/master/assets/config_screen_1.png">

### Passo 4

- Aperte em conectar e aparecerá a seguinte tela com alguns campos para serem preenchidos:

<img src="https://github.com/marceloams/smart-device/blob/master/assets/config_screen_2.png">

### Passo 5

### Antes de preencher

 - Abra o aplicativo mobile e siga os seguintes passos:
 
    - Adicione um dispositivo no aplicativo (Tela 1);
    - Clique em configurar (Icone de engrenagem, Tela 2);
    - Copie o id do dispositivo (Tela 3).
 
| **Tela 1** | **Tela 2** | **Tela 3** |
| :---: |:---:| :---:|
|![marceloamsProductions](https://github.com/marceloams/smart-device/blob/master/assets/app1.png)|![marceloamsProductions](https://github.com/marceloams/smart-device/blob/master/assets/app2.png) | ![marceloamsProductions](https://github.com/marceloams/smart-device/blob/master/assets/app3.png)
 
 ### Preenchendo os campos:

 - Escolha a rede ao qual deseja-se conectar e entre com a senha de acesso;
 - **Device Settings:** Entre com o id do dispositivo que foi obtido por meio do aplicativo mobile;
 - **Local Wifi Setting:** Pode editar o ssid e a senha do modo de ponto de acesso do dispositivo;
 - Após esses passos, deverá aparecer a seguinte tela:
 
<img src="https://github.com/marceloams/smart-device/blob/master/assets/config_screen_3.png">
 
 ## Passo 6
 
 - Após isto, espere alguns segundos e já terá carregado no app:
  
 <img src="https://github.com/marceloams/smart-device/blob/master/assets/app2.png">
 
 - Também será possível manipular o dipositivo além de acessar seu histórico de medidas.
 
 <img src="https://github.com/marceloams/smart-device/blob/master/assets/app4.png">
 
 ---
 
✔️ Se seguiu os passo e obteve os mesmos resultados é sinal que funcionou!
<br>
<br>
<img src="https://media.giphy.com/media/13G7hmmFr9yuxG/giphy.gif">
<br>
<br>
❌ Se algum dos passos não funcionou, algo de errado não está certo :( (Faça um issue)
<br>
<br>
<img src="https://media.giphy.com/media/12Bpme5pTzGmg8/giphy.gif">

---

## Contribuição

### Passo 1

- **Opção 1**
    - 🍴 Forkar esse repositório!

- **Opção 2**
    - 👯 Clonar para sua maquina atual.

### Passo 2

- **Codar!** 👨‍💻👩‍💻

### Passo 3

- 🔃 Crie um novo pull request.

---

## Bibliotecas utilizadas

  - <a href="https://github.com/tzapu/WiFiManager" target="_blank">Wifi Manager</a>;
  - <a href="https://github.com/bblanchon/ArduinoJson" target="_blank">Arduino Json</a>;
  - <a href="https://github.com/mobizt/Firebase-ESP8266" target="_blank">Firebase-Esp8266</a>;
  - <a href="https://github.com/mobizt/Firebase-ESP8266" target="_blank">FS.h</a>;
  - <a href="https://github.com/mobizt/Firebase-ESP8266" target="_blank">ESP8266WiFi.h</a>;
  - <a href="https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer" target="_blank">DNSServer</a>;
  - <a href="https://github.com/adafruit/DHT-sensor-library" target="_blank">DHT</a>;
  - <a href="https://github.com/arduino-libraries/NTPClient" target="_blank">NTPClient</a>;
  - <a href="https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/WiFiUdp.h" target="_blank">WiFiUdp</a>.
 
 ---
 
 ## Autor


| **Marcelo Amorim** |
| :---: |
| [![marceloamsProductions](https://avatars1.githubusercontent.com/u/63866348?&v=4&s=200)](https://github.com/marceloams) |

---
 
 ## Licença

[![License](http://img.shields.io/:license-mit-blue.svg?style=flat-square)](http://badges.mit-license.org)

- **[MIT license](http://opensource.org/licenses/mit-license.php)**
- Copyright 2020 © <a href="https://github.com/marceloams/smart-device" target="_blank">SmartDeviceProductions</a>.

---

