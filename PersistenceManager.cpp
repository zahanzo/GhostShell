#include <initguid.h>
#include "PersistenceManager.hpp"

_COM_SMARTPTR_TYPEDEF(ITaskService, __uuidof(ITaskService));
_COM_SMARTPTR_TYPEDEF(ITaskDefinition, __uuidof(ITaskDefinition));
_COM_SMARTPTR_TYPEDEF(ITaskFolder, __uuidof(ITaskFolder));
_COM_SMARTPTR_TYPEDEF(ITriggerCollection, __uuidof(ITriggerCollection));
_COM_SMARTPTR_TYPEDEF(ITrigger, __uuidof(ITrigger));
_COM_SMARTPTR_TYPEDEF(IActionCollection, __uuidof(IActionCollection));
_COM_SMARTPTR_TYPEDEF(IAction, __uuidof(IAction));
_COM_SMARTPTR_TYPEDEF(IExecAction, __uuidof(IExecAction));
_COM_SMARTPTR_TYPEDEF(IRegisteredTask, __uuidof(IRegisteredTask));

// NOVAS INTERFACES NECESSÁRIAS
_COM_SMARTPTR_TYPEDEF(ITaskSettings, __uuidof(ITaskSettings));
_COM_SMARTPTR_TYPEDEF(IPrincipal, __uuidof(IPrincipal));

bool PersistenceManager::CreateLogonTask(const std::wstring& taskName, const std::wstring& exePath) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) return false;

    try {
        ITaskServicePtr pService;
        hr = pService.CreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER);
        if (FAILED(hr)) throw hr;

        hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
        if (FAILED(hr)) throw hr;

        ITaskDefinitionPtr pTask;
        hr = pService->NewTask(0, &pTask);

        ITaskSettingsPtr pSettings;
        hr = pTask->get_Settings(&pSettings);
        if (SUCCEEDED(hr)) {
            pSettings->put_StartWhenAvailable(VARIANT_TRUE);
            pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE); // Ignora se estiver na bateria
            pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);     // Não mata o processo se tirar da tomada
            pSettings->put_Hidden(VARIANT_TRUE);                      // Esconde a tarefa da interface básica
            pSettings->put_ExecutionTimeLimit(_bstr_t(L"PT0S"));      // Tempo limite = 0 (Roda para sempre)
        }

        
        IPrincipalPtr pPrincipal;
        hr = pTask->get_Principal(&pPrincipal);
        if (SUCCEEDED(hr)) {
            pPrincipal->put_LogonType(TASK_LOGON_INTERACTIVE_TOKEN);
            pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST); // Pede os maiores privilégios disponíveis
        }

        ITriggerCollectionPtr pTriggerCollection;
        hr = pTask->get_Triggers(&pTriggerCollection);
        
        ITriggerPtr pTrigger;
        hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);

        IActionCollectionPtr pActionCollection;
        hr = pTask->get_Actions(&pActionCollection);
        
        IActionPtr pAction;
        hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);

        IExecActionPtr pExecAction = pAction; 
        pExecAction->put_Path(_bstr_t(exePath.c_str()));

        ITaskFolderPtr pRootFolder;
        hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);

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
        CoUninitialize();
        return false;
    }

    CoUninitialize(); 
    return true;
}