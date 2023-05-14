#include "Alloc_OverflowCheck.h"

#include <Windows.h>

#include <assert.h>

#include <list>

constexpr int PAGE_SIZE = 4096;
constexpr int RESERVE_SIZE = (1024 * 64);
constexpr int PAGES_PER_1RESERVED = (RESERVE_SIZE / PAGE_SIZE);

struct AllocInfo
{
    void* pBase;
    BYTE usedPagesArr[RESERVE_SIZE / PAGE_SIZE];
};

class AllocManager
{
public:
    AllocManager()
    {
        NewVirtualAlloc();
    }

    ~AllocManager()
    {
        for (AllocInfo* pAllocInfo : mAllocInfoList)
            delete pAllocInfo;
    }

    void* Alloc(size_t size)
    {
        if (size >= RESERVE_SIZE)
            return nullptr;

        size_t requestSize = ((size - 1) / PAGE_SIZE) * PAGE_SIZE + PAGE_SIZE;
        size_t totalSize = requestSize + PAGE_SIZE;

        AllocInfo* pAllocInfo = mAllocInfoList.back();
        size_t pageIdx = GetAvailablePageIdx(pAllocInfo->usedPagesArr, totalSize / PAGE_SIZE);

        void* pRet;

        if (pageIdx >= 0)
        {
            UINT_PTR commitPtr = (UINT_PTR)pAllocInfo->pBase + PAGE_SIZE * pageIdx;

            pRet = VirtualAlloc((void*)commitPtr, totalSize, MEM_COMMIT, PAGE_READWRITE);

            VirtualAlloc((void*)(commitPtr + requestSize), PAGE_SIZE, MEM_COMMIT, PAGE_NOACCESS);

            for (int cnt = 0; cnt < totalSize / PAGE_SIZE; ++cnt)
            {
                pAllocInfo->usedPagesArr[pageIdx + cnt] = (BYTE)(totalSize / PAGE_SIZE);
            }
        }
        else
        {
            NewVirtualAlloc();

            UINT_PTR commitPtr = (UINT_PTR)mAllocInfoList.back();
            pRet = VirtualAlloc((void*)commitPtr, totalSize, MEM_COMMIT, PAGE_READWRITE);

            VirtualAlloc((void*)(commitPtr + requestSize), PAGE_SIZE, MEM_COMMIT, PAGE_NOACCESS);

            for (int cnt = 0; cnt < totalSize / PAGE_SIZE; ++cnt)
            {
                pAllocInfo->usedPagesArr[cnt] = (BYTE)(totalSize / PAGE_SIZE);
            }
        }

        pRet = (void*)((UINT_PTR)pRet + requestSize - size);

        return pRet;
    }

    BOOL Free(void* p)
    {
        UINT_PTR reservedPtr = (UINT_PTR)p & ~(UINT_PTR)(RESERVE_SIZE - 1);
        UINT_PTR commitPtr = (UINT_PTR)p & ~(UINT_PTR)(PAGE_SIZE - 1);
        size_t pageIdx = (commitPtr - reservedPtr) / PAGE_SIZE;

        std::list<AllocInfo*>::iterator iter;
        for (iter = mAllocInfoList.begin(); iter != mAllocInfoList.end(); ++iter)
        {
            if ((*iter)->pBase == (void*)reservedPtr)
                break;
        }
        AllocInfo* pAllocInfo = *iter;

        size_t commitPages = pAllocInfo->usedPagesArr[pageIdx];

        for (size_t cnt = 0; cnt < commitPages; ++cnt)
        {
            pAllocInfo->usedPagesArr[pageIdx + cnt] = 0;
        }

        if (!VirtualAlloc((void*)commitPtr, commitPages * PAGE_SIZE, MEM_DECOMMIT, PAGE_NOACCESS))
            return FALSE;

        bool bCommit = false;
        for (int i = 0; i < RESERVE_SIZE / PAGE_SIZE; ++i)
        {
            if (pAllocInfo->usedPagesArr[i] != 0)
            {
                bCommit = true;
                break;
            }
        }

        if (!bCommit)
        {
            mAllocInfoList.erase(iter);
            return VirtualFree((void*)reservedPtr, 0, MEM_RELEASE);
        }

        return TRUE;
    }

    void NewVirtualAlloc()
    {
        UINT_PTR ptr;
        if (!mAllocInfoList.empty())
        {
            void* pPrevBase = mAllocInfoList.back();
            ptr = (UINT_PTR)pPrevBase + RESERVE_SIZE;
        }
        else
            ptr = 0;

        void* p = VirtualAlloc((void*)ptr, RESERVE_SIZE, MEM_RESERVE, PAGE_NOACCESS);
        if (p == nullptr)
            p = VirtualAlloc(NULL, RESERVE_SIZE, MEM_RESERVE, PAGE_NOACCESS);

        AllocInfo* pAllocInfo = new AllocInfo;
        pAllocInfo->pBase = p;
        ZeroMemory(pAllocInfo->usedPagesArr, sizeof(AllocInfo::usedPagesArr));

        mAllocInfoList.push_back(pAllocInfo);
    }

    size_t GetAvailablePageIdx(const BYTE* usedPagesArr, size_t needPages)
    {
        size_t count = 0;
        size_t baseIdx = 0;
        bool bPrevAvailable = false;

        for (int i = 0; i < RESERVE_SIZE / PAGE_SIZE; ++i)
        {
            if (usedPagesArr[i])
            {
                bPrevAvailable = false;
            }
            else if (!bPrevAvailable)
            {
                bPrevAvailable = true;
                baseIdx = i;
            }

            if (bPrevAvailable)
            {
                ++count;
                if (count >= needPages)
                    return baseIdx;
            }
            else
            {
                count = 0;
            }
        }

        return -1;
    }

private:
    std::list<AllocInfo*> mAllocInfoList;
};

static AllocManager _AllocManager;

void* Alloc_OverflowCheck(size_t size)
{
    void* pAlloc = _AllocManager.Alloc(size);

    return pAlloc;
}

bool Free_OverflowCheck(void* p)
{
    BOOL bResult = _AllocManager.Free(p);

    return bResult;
}
