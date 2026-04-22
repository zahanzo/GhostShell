# 🛡️ GhostShell: Toolkit Avançado de Evasão e Persistência

Este repositório contém uma Prova de Conceito (PoC) para um Reverse Shell altamente furtivo e *fileless* (sem arquivos) desenvolvido em **C++20**. Projetado como uma ferramenta de pesquisa em segurança ofensiva, este projeto demonstra uma interação avançada com a API do Windows (Win32 e COM), focando em execução em memória, evasão de *sandbox* e persistência com privilégios elevados.

> ⚠️ **Aviso de Segurança e Ética:** Este projeto foi criado estritamente para fins **educacionais e de pesquisa em Segurança da Informação**. O uso desta ferramenta em sistemas sem consentimento prévio é ilegal. O foco aqui é o estudo de arquitetura de software e defesa cibernética (Blue Team/Red Team).

---

## ⚙️ Fluxo de Execução (The Kill Chain)

O shell reverso segue um *pipeline* de execução estrito e passo a passo, projetado para contornar heurísticas estáticas e análises dinâmicas (Sandboxes).

### Passo 1: Ofuscação de Interface (Ocultação da UI)
Imediatamente após a execução, o binário oculta o seu próprio contexto de execução. Ele consulta a janela ativa usando `GetConsoleWindow()` e força uma chamada `ShowWindow(hWnd, SW_HIDE)`. Isso impede que o subsistema do console pisque na tela do alvo, garantindo uma inicialização silenciosa.

### Passo 2: Mapeamento de Ambiente (Anti-Sandbox)
Antes de estabelecer qualquer conexão de rede, o shell reverso inspeciona o ambiente hospedeiro para detectar virtualização ou ferramentas de análise automatizada:
* **Checagens de Hardware:** Usa `GlobalMemoryStatusEx()` e `GetSystemInfo()` para verificar se o sistema possui pelo menos 4GB de RAM e múltiplos núcleos de CPU.
* **Enumeração de Drivers:** Verifica o sistema de arquivos em busca de artefatos conhecidos de *hypervisors* (ex: `VBoxMouse.sys`, `vmhgfs.sys`). 
Se qualquer checagem falhar, o processo é encerrado silenciosamente (`return 0`).

### Passo 3: Evasão Temporal (Heavy Delay)
Para contornar *sandboxes* de análise dinâmica que possuem limites de tempo de execução estritos (geralmente de 2 a 3 minutos), o shell reverso executa um loop matemático determinístico e de uso intensivo de CPU. Isso esgota o cronômetro de monitoramento da *sandbox* antes que a carga maliciosa real (*payload*) seja desempacotada.

### Passo 4: Persistência baseada em COM
Em vez de modificar a chave de registro altamente monitorada `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`, o shell reverso se comunica diretamente com a **API COM do Task Scheduler 2.0**:
* Inicializa o COM via `CoInitializeEx`.
* Solicita as interfaces `ITaskService` e `IPrincipal`.
* Cria uma tarefa agendada oculta acionada por `TASK_TRIGGER_LOGON`.
* Eleva o contexto de execução solicitando `TASK_RUNLEVEL_HIGHEST`, ignorando completamente os monitores de inicialização padrão e garantindo que o shell sobreviva a reinicializações com privilégios máximos.

### Passo 5: Recuperação e Descriptografia de C2 Fileless
A URL de Comando e Controle (C2) é protegida contra a extração estática de *strings* (como o uso de `strings.exe` ou regras YARA) usando um *template* customizado `constexpr` que criptografa a URL via XOR em **tempo de compilação** (*compile-time*).
Em tempo de execução (*runtime*), a URL é descriptografada na memória. A biblioteca `WinInet` (`InternetOpenUrlA`, `InternetReadFile`) é então usada para buscar o IP alvo e a Porta dinamicamente, armazenando-os estritamente na RAM (*Fileless*), sem tocar no disco.

### Passo 6: Sequestro de Handles e Reverse Shell Nativo
A fase final estabelece a conexão e concede controle remoto:
* Inicializa a pilha de rede via `WSAStartup`.
* Cria um *socket* TCP sobreposto usando `WSASocketA`.
* Converte a *string* do IP para uma estrutura binária de ordem de rede usando a API moderna `inet_pton`.
* Executa uma nova instância do `cmd.exe` via `CreateProcessA`. Crucialmente, ele manipula a estrutura `STARTUPINFO` definindo a *flag* `STARTF_USESTDHANDLES`. Isso vincula os identificadores (*handles*) de Entrada Padrão (`STDIN`), Saída (`STDOUT`) e Erro (`STDERR`) diretamente ao *handle* aberto do Winsock. 
* O resultado é um shell remoto totalmente interativo sem depender de binários externos como o `nc.exe`.

---

### 📡 Configuração Remota do C2 (Exemplo no Pastebin)

O módulo `DownloadManager` foi projetado para buscar o IP alvo e a Porta dinamicamente de um servidor remoto. O *parser* no `main.cpp` divide estritamente a *string* baixada usando o caractere *pipe* (`|`).

**Estrutura de Arquivo Obrigatória:**
O arquivo de texto remoto deve conter exatamente o endereço IP e a porta, separados por um único *pipe*. Ele não deve conter **nenhum** espaço extra, protocolos (como `tcp://`) ou quebras de linha no final.

**Formato:**
'''text
<IP_ADDRESS>|<PORT>
'''

**Exemplo de Implantação no Pastebin:**
1. Crie um novo *paste* contendo apenas o IP e a Porta do seu ouvinte (ex: seu IP do Ngrok ou IP local do Kali):
'''text
3.14.15.92|12345
'''
2. Salve o *paste* e copie o link **Raw** (Cru) (ex: `https://pastebin.com/raw/XYZ123`).
3. Atualize a macro `PROTECT("...")` no `main.cpp` com esta **URL Raw** antes de compilar. 

*Nota: Se a URL fornecida não for a versão Raw, a resposta HTTP do `WinInet` fará o download de toda a estrutura da página HTML, o que fará com que o parser `std::stoi` falhe silenciosamente durante a execução.*

---

## 💻 Compilação e Implantação (Deployment)

Para manter o seu perfil furtivo e evitar o uso de dependências externas (como `vcruntime` ou `libstdc++`), compile o binário estaticamente usando o MinGW-w64 (GCC) com o padrão C++20 ativado:

'''bash
g++ main.cpp AntiSandbox.cpp DownloadManager.cpp PersistenceManager.cpp -o win_service.exe -std=c++20 -static -static-libgcc -static-libstdc++ -mwindows -s -lws2_32 -lwininet -lole32 -loleaut32 -luuid
'''

**Detalhamento das Flags do Compilador:**
* `-std=c++20`: Necessário para a ofuscação de *string* XOR em tempo de compilação (`constexpr`).
* `-static`: Embuti todas as bibliotecas padrão do C++ diretamente no binário.
* `-mwindows`: Instrui o subsistema do Windows a tratar o executável como um aplicativo com interface gráfica, impedindo nativamente que a janela do console CMD apareça.
* `-s`: Remove todos os símbolos de depuração (*debug*), reduzindo significativamente o tamanho do binário e dificultando os esforços de engenharia reversa.