#pragma once

void* Alloc_OverflowCheck(size_t size);
bool Free_OverflowCheck(void* p);