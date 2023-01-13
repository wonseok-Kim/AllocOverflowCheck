#pragma once

void* Alloc_OverflowCheck(size_t size);
bool Free_OverflowCheck(void* p);

//void* Alloc_OverflowCheck(size_t size)
//{
//	size_t requestSize = ((size - 1) / PAGE_SIZE) * PAGE_SIZE + PAGE_SIZE;
//	size_t totalSize = requestSize + PAGE_SIZE;
//
//	BYTE* p = (BYTE*)VirtualAlloc(NULL, totalSize, MEM_COMMIT, PAGE_READWRITE);
//	VirtualAlloc(p + requestSize, PAGE_SIZE, MEM_COMMIT, PAGE_NOACCESS);
//
//	BYTE* pRet = p + requestSize - size;
//	return pRet;
//}

//BOOL Free_OverflowCheck(void* p)
//{
//	 UINT_PTR srcPtr = (UINT_PTR)p & ~(UINT_PTR)(PAGE_SIZE - 1);
//
//	 return VirtualFree((void*)srcPtr, 0, MEM_RELEASE);
//}
