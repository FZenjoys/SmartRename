#pragma once
#include <vector>
#include <map>
#include "srwlock.h"

class CSmartRenameManager :
    public ISmartRenameManager,
    public ISmartRenameRegExEvents
{
public:
    // IUnknown
    IFACEMETHODIMP  QueryInterface(_In_ REFIID iid, _Outptr_ void** resultInterface);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // ISmartRenameManager
    IFACEMETHODIMP Advise(_In_ ISmartRenameManagerEvents* renameOpEvent, _Out_ DWORD *cookie);
    IFACEMETHODIMP UnAdvise(_In_ DWORD cookie);
    IFACEMETHODIMP Start();
    IFACEMETHODIMP Stop();
    IFACEMETHODIMP Reset();
    IFACEMETHODIMP Shutdown();
    IFACEMETHODIMP Rename(_In_ HWND hwndParent);
    IFACEMETHODIMP AddItem(_In_ ISmartRenameItem* pItem);
    IFACEMETHODIMP GetItemByIndex(_In_ UINT index, _COM_Outptr_ ISmartRenameItem** ppItem);
    IFACEMETHODIMP GetItemById(_In_ int id, _COM_Outptr_ ISmartRenameItem** ppItem);
    IFACEMETHODIMP GetItemCount(_Out_ UINT* count);
    IFACEMETHODIMP GetSelectedItemCount(_Out_ UINT* count);
    IFACEMETHODIMP GetRenameItemCount(_Out_ UINT* count);
    IFACEMETHODIMP get_flags(_Out_ DWORD* flags);
    IFACEMETHODIMP put_flags(_In_ DWORD flags);
    IFACEMETHODIMP get_renameRegEx(_COM_Outptr_ ISmartRenameRegEx** ppRegEx);
    IFACEMETHODIMP put_renameRegEx(_In_ ISmartRenameRegEx* pRegEx);
    IFACEMETHODIMP get_renameItemFactory(_COM_Outptr_ ISmartRenameItemFactory** ppItemFactory);
    IFACEMETHODIMP put_renameItemFactory(_In_ ISmartRenameItemFactory* pItemFactory);

    // ISmartRenameRegExEvents
    IFACEMETHODIMP OnSearchTermChanged(_In_ PCWSTR searchTerm);
    IFACEMETHODIMP OnReplaceTermChanged(_In_ PCWSTR replaceTerm);
    IFACEMETHODIMP OnFlagsChanged(_In_ DWORD flags);

    static HRESULT s_CreateInstance(_Outptr_ ISmartRenameManager** ppsrm);

protected:
    CSmartRenameManager();
    virtual ~CSmartRenameManager();

    HRESULT _Init();
    void _Cleanup();

    void _Cancel();

    void _OnItemAdded(_In_ ISmartRenameItem* renameItem);
    void _OnUpdate(_In_ ISmartRenameItem* renameItem);
    void _OnError(_In_ ISmartRenameItem* renameItem);
    void _OnRegExStarted(_In_ DWORD threadId);
    void _OnRegExCanceled(_In_ DWORD threadId);
    void _OnRegExCompleted(_In_ DWORD threadId);
    void _OnRenameStarted();
    void _OnRenameCompleted();

    void _ClearEventHandlers();
    void _ClearSmartRenameItems();

    HRESULT _PerformRegExRename();
    HRESULT _PerformFileOperation();
 
    HRESULT _CreateRegExWorkerThread();
    void _CancelRegExWorkerThread();
    void _WaitForRegExWorkerThread();
    HRESULT _CreateFileOpWorkerThread();

    HRESULT _EnsureRegEx();
    HRESULT _InitRegEx();
    void _ClearRegEx();

    // Thread proc for performing the regex rename of each item
    static DWORD WINAPI s_regexWorkerThread(_In_ void* pv);
    // Thread proc for performing the actual file operation that does the file rename
    static DWORD WINAPI s_fileOpWorkerThread(_In_ void* pv);

    static LRESULT CALLBACK s_msgWndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
    LRESULT _WndProc(_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    HANDLE m_regExWorkerThreadHandle = nullptr;
    HANDLE m_startRegExWorkerEvent = nullptr;
    HANDLE m_cancelRegExWorkerEvent = nullptr;

    HANDLE m_fileOpWorkerThreadHandle = nullptr;
    HANDLE m_startFileOpWorkerEvent = nullptr;

    CSRWLock m_lockEvents;
    CSRWLock m_lockItems;

    DWORD m_flags = 0;

    DWORD m_cookie = 0;
    DWORD m_regExAdviseCookie = 0;

    struct RENAME_MGR_EVENT
    {
        ISmartRenameManagerEvents* pEvents;
        DWORD cookie;
    };

    CComPtr<ISmartRenameItemFactory> m_spItemFactory;
    CComPtr<ISmartRenameRegEx> m_spRegEx;

    _Guarded_by_(m_lockEvents) std::vector<RENAME_MGR_EVENT> m_renameManagerEvents;
    _Guarded_by_(m_lockItems) std::map<int, ISmartRenameItem*> m_renameItems;

    // Parent HWND used by IFileOperation
    HWND m_hwndParent = nullptr;

    HWND m_hwndMessage = nullptr;

    CRITICAL_SECTION m_critsecReentrancy;

    long m_refCount;
};