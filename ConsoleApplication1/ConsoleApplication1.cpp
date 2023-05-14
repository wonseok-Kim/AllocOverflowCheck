#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "Alloc_OverflowCheck.h"

void Fill_Overflow(void* ptr, int val, size_t size)
{
    size_t i;
    BYTE* write = (BYTE*)ptr;

    for (i = 0; i < size; ++i)
    {
        *(write + i) = val;
    }

    *(write + i) = val;
}

void Fill(void* ptr, int val, size_t size)
{
    size_t i;
    BYTE* write = (BYTE*)ptr;

    for (i = 0; i < size; ++i)
    {
        *(write + i) = val;
    }
}

int main()
{
    BYTE* a = (BYTE*)Alloc_OverflowCheck(10);
    Fill(a, 0x11, 10);
    Free_OverflowCheck(a);

    //BYTE* b = (BYTE*)(Alloc_OverflowCheck(4095));
    //Fill(b, 0x22, 4095);
    //Free_OverflowCheck(b);

    //BYTE* c = (BYTE*)(Alloc_OverflowCheck(4096));
    //Fill(c, 0x33, 4096);
    //Free_OverflowCheck(c);

    //BYTE* d = (BYTE*)(Alloc_OverflowCheck(4097));
    //Fill(d, 0x44, 4097);
    //Free_OverflowCheck(d);

    //BYTE* e = (BYTE*)(Alloc_OverflowCheck(4096 * 2 + 1));
    //Fill(e, 0x55, 4096 * 2 + 1);
    //Free_OverflowCheck(e);

    //BYTE* f = (BYTE*)(Alloc_OverflowCheck(10));
    //Fill(f, 0x66, 10);
    //Free_OverflowCheck(f);

    {
        //int* p1 = (int*)Alloc_OverflowCheck(PAGE_SIZE - 1);
        //int* p2 = (int*)Alloc_OverflowCheck(PAGE_SIZE);
        //int* p3 = (int*)Alloc_OverflowCheck(PAGE_SIZE + 1);

        //int* p4 = (int*)Alloc_OverflowCheck(PAGE_SIZE * 2 - 1);
        //int* p5 = (int*)Alloc_OverflowCheck(PAGE_SIZE * 2);
        //int* p6 = (int*)Alloc_OverflowCheck(PAGE_SIZE * 2 + 1);
    }

    //{
    //	UINT_PTR ptr;
    //	void* p1 = VirtualAlloc(NULL, 4096 * 16, MEM_RESERVE, PAGE_NOACCESS);

    //	ptr = (size_t)p1 + 4096 * 16;
    //	void* p2 = VirtualAlloc((void*)ptr, 4096 * 16, MEM_RESERVE, PAGE_NOACCESS);


    //	ptr = (size_t)p1 + 4096 * 15;
    //	void* p3 = VirtualAlloc((void*)ptr, 4096 + 1, MEM_COMMIT, PAGE_READWRITE);


    //	return 0;
    //}

    {
        std::vector<void*> ptrs;
        ptrs.reserve(1000);

        MEMORY_BASIC_INFORMATION mbi;
        
        DWORD memCommit = MEM_COMMIT;
        DWORD memReserve = MEM_RESERVE;
        DWORD memFree = MEM_FREE;

        UINT_PTR reservePtr1;
        UINT_PTR reservePtr2;

        for (int i = 0; i < 1000; ++i)
        {
            void* p1 = malloc(sizeof(int) * 100);
            Fill(p1, 0x11, sizeof(int)* 100);
            ptrs.push_back(p1);
            
            if (i == 0)
            {
                reservePtr1 = (UINT_PTR(p1) & ~UINT_PTR(1024 * 64 - 1)) + UINT_PTR(1024 * 64 * 2);
                reservePtr2 = (UINT_PTR(p1) & ~UINT_PTR(1024 * 64 - 1)) + UINT_PTR(1024 * 64 * 3);
            }
            
            
        }

        SIZE_T ret = VirtualQuery((PVOID)reservePtr1, &mbi, sizeof(mbi));
        if (ret != 0)
        {
            if (mbi.State == memCommit)
            {
                printf("reservePtr1 state is commit\n");
            }
        }

        ret = VirtualQuery((void*)reservePtr2, &mbi, sizeof(mbi));
        if (ret != 0)
        {
            if (mbi.State == memCommit)
            {
                printf("reservePtr2 state is commit\n");
            }
        }


        for (int i = 0; i < 1000; ++i)
        {
            void* p2 = ptrs[i];            
            free(p2);

            ret = VirtualQuery((PVOID)reservePtr1, &mbi, sizeof(mbi));
            if (ret != 0)
            {
                if (mbi.State != memCommit)
                {
                    printf("reservePtr1 state is not commit\n");
                }
            }

            ret = VirtualQuery((void*)reservePtr2, &mbi, sizeof(mbi));
            if (ret != 0)
            {
                if (mbi.State != memCommit)
                {
                    printf("reservePtr2 state is not commit\n");
                }
            }

        }

    }

    return 0;
}